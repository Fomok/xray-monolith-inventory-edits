#pragma once
#include "placement.h"
#include "transfer_session.h"
#include "world_subtrees.h"
#include <memory>

namespace inventory_next {
// Single-authority actor <-> native world-inventory transaction. The caller
// supplies freshly verified engine facts and rechecks access before dispatch.
// Ground bags need a distinct native custody representation and are not admitted
// by pretending that all descendants are unrelated ground objects.
class LoadedStorageSession {
public:
    LoadedStorageSession(const Model& actor,const WorldSubtrees& world,const EntityRegistry& registry)
        : actor_(actor),world_(world),registry_(registry) {}
    Result prepare_release(Id shell,Id custodian) { return prepare(shell,custodian,false,{}); }
    Result prepare_acquire(Id shell,Id custodian,const std::vector<Id>& destinations) {
        return prepare(shell,custodian,true,destinations);
    }
    Result dispatch(bool accessible,TransferSession::Dispatch& output) {
        if(!transfer_||committed_||transfer_->state()!=TransferSession::State::Prepared)return {false,"transfer_state"};
        auto result=current();
        if(!accessible||!result) {
            transfer_->cancel();transfer_.reset();return accessible?result:Result{false,"world_unavailable"};
        }
        return transfer_->dispatch(output);
    }
    Result acknowledge(std::uint64_t token,const std::vector<TransferSession::ParentObservation>& parents,bool accepted) {
        if(!transfer_||committed_)return {false,"transfer_state"};
        auto result=current();
        if(!result){fault_=true;return result;}
        result=transfer_->acknowledge_subtree(token,parents,accepted);
        if(!result)return result;
        committed_=true;return {true,{}};
    }
    bool can_save() const { return !fault_&&(!transfer_||transfer_->can_save()); }
    const Model* committed_actor() const { return committed_?&actor_candidate_:nullptr; }
    const WorldSubtrees* committed_world() const { return committed_?&world_candidate_:nullptr; }
private:
    Result current() const {
        if(actor_.session()!=actor_session_||actor_.revision()!=actor_revision_||actor_.serialize()!=actor_before_)return {false,"stale_model"};
        if(world_.serialize()!=world_before_)return {false,"stale_world_storage"};
        return {true,{}};
    }
    Result prepare(Id shell,Id custodian,bool acquire,const std::vector<Id>& destinations) {
        if(transfer_||fault_||committed_)return {false,"transfer_busy"};
        if(custodian<100||!registry_.handle(custodian)||actor_.nodes().count(custodian))return {false,"world_storage_custodian"};
        if(!actor_.nodes().count(1)||!actor_.nodes().count(2))return {false,"native_layout_root"};
        std::vector<Node> nodes;
        for(const auto& pair:actor_.nodes()) {
            if(pair.second.kind==Kind::Item) {
                Id owner=pair.first;while(actor_.nodes().at(owner).parent)owner=actor_.nodes().at(owner).parent;
                if(owner!=1)return {false,"native_layout_owner"};
            }
            nodes.push_back(pair.second);
        }
        Node owner;owner.id=custodian;owner.kind=Kind::WorldRoot;owner.category=0;owner.container=true;owner.grid={64,64};nodes.push_back(owner);
        if(acquire) {
            const auto record=world_.records().find(shell);
            if(record==world_.records().end()||record->second.custodian!=custodian)return {false,"world_storage_custodian"};
            for(const auto& pair:record->second.tree.nodes())if(pair.second.kind==Kind::Item) {
                auto n=pair.second;if(n.id==shell)n.parent=custodian;nodes.push_back(std::move(n));
            }
        } else {
            const auto item=actor_.nodes().find(shell);
            if(item==actor_.nodes().end()||!item->second.container)return {false,"world_storage_shell"};
        }
        auto result=staged_.initialize(nodes);if(!result)return result;
        Command command;command.item=shell;command.destination=custodian;command.session=staged_.session();command.revision=staged_.revision();
        if(acquire) {
            for(Id destination:destinations) {
                auto found=staged_.nodes().find(destination);if(found==staged_.nodes().end())return {false,"native_layout_owner"};
                Id root=destination;while(staged_.nodes().at(root).parent)root=staged_.nodes().at(root).parent;
                if(root!=1)return {false,"native_layout_owner"};
            }
            auto plan=find_placement(staged_,shell,destinations);if(!plan.result)return plan.result;
            command.destination=plan.moves.front().destination;command.position=plan.moves.front().position;
        }
        Model completed=staged_;result=completed.execute(command);if(!result)return result;
        auto next_world=world_;
        result=acquire?next_world.release(shell,custodian):next_world.capture(completed,shell,custodian);
        if(!result)return result;
        std::vector<Node> actor_nodes;
        for(const auto& pair:completed.nodes()) {
            Id root=pair.first;while(completed.nodes().at(root).parent)root=completed.nodes().at(root).parent;
            if(root==1||pair.first==2)actor_nodes.push_back(pair.second);
        }
        Model next_actor;result=next_actor.initialize(actor_nodes);if(!result)return result;
        access_=std::make_unique<AccessGate>(staged_);result=access_->observe(custodian,true,true);if(!result)return result;
        if(acquire){result=access_->observe_item(shell,true,true);if(!result)return result;}
        auto transfer=std::make_unique<TransferSession>(staged_,*access_,registry_);
        TransferSession::PhysicalOwners physical;physical.flatten_source=true;physical.flatten_destination=true;
        physical.source=registry_.handle(acquire?custodian:Id{1});physical.destination=registry_.handle(acquire?Id{1}:custodian);
        result=transfer->prepare({command,access_->revision()},false,physical);if(!result)return result;
        actor_candidate_=std::move(next_actor);world_candidate_=std::move(next_world);
        actor_before_=actor_.serialize();world_before_=world_.serialize();actor_session_=actor_.session();actor_revision_=actor_.revision();
        transfer_=std::move(transfer);return {true,{}};
    }
    const Model& actor_;const WorldSubtrees& world_;const EntityRegistry& registry_;
    Model staged_,actor_candidate_;WorldSubtrees world_candidate_;
    std::unique_ptr<AccessGate> access_;std::unique_ptr<TransferSession> transfer_;
    std::string actor_before_,world_before_;std::uint64_t actor_session_=0,actor_revision_=0;
    bool committed_=false,fault_=false;
};
}
