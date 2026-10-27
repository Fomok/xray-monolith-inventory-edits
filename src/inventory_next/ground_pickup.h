#pragma once
#include "placement.h"
#include "transfer_session.h"
#include <memory>
namespace inventory_next {
// One synchronous server ownership transaction. The authority publishes model()
// only after a matching native parent acknowledgement. Published arrivals reserve
// their cells while the client catches up; they are not ordinary recovery items.
class GroundPickup {
public:
    explicit GroundPickup(const EntityRegistry& registry):registry_(registry) {}
    Result prepare(const Model& carried,Node incoming,const std::vector<Id>& destinations,bool ground_source=true) {
        if(transfer_)return {false,"transfer_busy"};
        if(carried.nodes().count(incoming.id))return {false,"duplicate_item"};
        std::vector<Node> nodes;
        for(const auto& pair:carried.nodes()) {
            if(pair.second.kind==Kind::Item&&pair.second.parent==2)return {false,"world_not_empty"};
            nodes.push_back(pair.second);
        }
        incoming.parent=2;incoming.location=Location::Grid;incoming.position={};incoming.equipped_slots=0;
        // The world root is a transient staging surface, never actor capacity.
        for(auto& n:nodes)if(n.id==2)n.grid={64,64};
        nodes.push_back(incoming);
        auto result=model_.initialize(nodes);if(!result)return result;
        const auto plan=find_placement(model_,incoming.id,destinations);
        if(!plan.result)return plan.result;
        access_=std::make_unique<AccessGate>(model_);
        result=access_->observe(2,true,true);if(!result)return result;
        result=access_->observe_item(incoming.id,true,true);if(!result)return result;
        const auto actor=registry_.handle(1);if(!actor)return {false,"offline_actor"};
        Command command;command.item=incoming.id;command.destination=plan.moves[0].destination;
        command.position=plan.moves[0].position;command.session=model_.session();command.revision=model_.revision();
        transfer_=std::make_unique<TransferSession>(model_,*access_,registry_);
        TransferSession::PhysicalOwners physical;physical.destination=actor;
        result=transfer_->prepare({command,access_->revision()},ground_source,physical);
        if(!result){transfer_.reset();return result;}
        return transfer_->dispatch(dispatch_);
    }
    Result acknowledge(std::uint16_t parent,bool accepted) {
        if(!transfer_)return {false,"transfer_state"};
        return transfer_->acknowledge(dispatch_.token,parent,accepted);
    }
    bool can_save()const{return !transfer_||transfer_->can_save();}
    const Model& model()const{return model_;}
private:
    const EntityRegistry& registry_;
    Model model_;
    std::unique_ptr<AccessGate> access_;
    std::unique_ptr<TransferSession> transfer_;
    TransferSession::Dispatch dispatch_;
};
}
