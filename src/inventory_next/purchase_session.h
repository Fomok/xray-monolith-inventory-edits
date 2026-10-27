#pragma once
#include "entity_registry.h"
#include "placement.h"
#include <limits>
#include <optional>
namespace inventory_next {
// Batch capacity and settlement coordinator. The native adapter must validate
// source access, prices and actual parents before prepare and again at dispatch.
// No item or money side effects are performed by this value-model coordinator.
class PurchaseSession {
public:
    enum class State { Idle, Prepared, Dispatched, AwaitingSettlement, Complete, ReconcileRequired };
    struct Line { Node item; std::uint32_t price=0; };
    struct Money { std::uint32_t actor=0, seller=0; };
    PurchaseSession(const Model& carried,const EntityRegistry& registry):carried_(carried),registry_(registry) {}
    Result prepare(const std::vector<Line>& lines,const std::vector<Id>& destinations,Money money) {
        if(state_!=State::Idle)return {false,"purchase_busy"};
        if(lines.empty())return {false,"empty_purchase"};
        auto actor=registry_.handle(1),seller=registry_.handle(2);
        if(!actor||!seller)return {false,"offline_owner"};
        for(Id destination:destinations) {
            auto node=carried_.nodes().find(destination);
            if(node==carried_.nodes().end()||!node->second.container||!carried_.storage_active(destination))return {false,"purchase_destination"};
            Id ancestor=destination;
            while(ancestor!=1) {
                const auto parent=carried_.nodes().find(ancestor);
                if(parent==carried_.nodes().end()||!parent->second.parent)return {false,"purchase_destination"};
                ancestor=parent->second.parent;
            }
        }
        Model candidate=carried_;std::vector<EntityRegistry::Handle> handles{*actor,*seller};
        std::map<Id,std::uint16_t> items;std::uint64_t total=0;
        for(const auto& pair:carried_.nodes())if(pair.second.kind==Kind::Item) {
            if(pair.second.parent==2)return {false,"world_not_empty"};
            auto handle=registry_.handle(pair.first);if(!handle)return {false,"offline_item"};
            handles.push_back(*handle);
        }
        for(const auto& line:lines) {
            const auto& incoming=line.item;
            if(candidate.nodes().count(incoming.id))return {false,"duplicate_item"};
            auto handle=registry_.handle(incoming.id);if(!handle)return {false,"offline_item"};
            handles.push_back(*handle);items.emplace(incoming.id,handle->live);
            total+=line.price;
            if(total>money.actor)return {false,"insufficient_money"};
            if(total>std::numeric_limits<std::uint32_t>::max()-money.seller)return {false,"money_overflow"};
            std::vector<Node> nodes;for(const auto& pair:candidate.nodes())nodes.push_back(pair.second);
            for(auto& node:nodes)if(node.id==2)node.grid={64,64};
            Node staged=incoming;staged.parent=2;staged.location=Location::Grid;staged.position={};staged.equipped_slots=0;
            nodes.push_back(staged);Model next;auto result=next.initialize(nodes);if(!result)return result;
            const auto plan=find_placement(next,staged.id,destinations);if(!plan.result)return plan.result;
            result=next.move(plan.moves.at(0));if(!result)return result;
            candidate=std::move(next);
        }
        before_=money;after_={static_cast<std::uint32_t>(money.actor-total),static_cast<std::uint32_t>(money.seller+total)};
        revision_=carried_.revision();session_=carried_.session();handles_=std::move(handles);items_=std::move(items);
        actor_=actor->live;seller_=seller->live;candidate_=std::move(candidate);state_=State::Prepared;
        return {true,{}};
    }
    Result dispatch(bool source_access,Money money) {
        if(state_!=State::Prepared)return {false,"purchase_state"};
        auto result=current();if(!result)return result;
        if(!source_access)return {false,"source_access"};
        if(!same(money,before_))return {false,"money_changed"};
        state_=State::Dispatched;return {true,{}};
    }
    // Read actual native parents for the exact persistent identities. All-or-none
    // native rejection is saveable; partial movement requires reconciliation.
    Result acknowledge(const std::map<Id,std::uint16_t>& parents,bool accepted,Money money) {
        if(state_!=State::Dispatched)return {false,"purchase_state"};
        auto result=current();if(!result){state_=State::ReconcileRequired;return result;}
        bool match=parents.size()==items_.size();
        for(const auto& item:items_) {
            const auto observed=parents.find(item.first);
            match=match&&observed!=parents.end()&&observed->second==(accepted?actor_:seller_);
        }
        if(!match||!same(money,before_)){state_=State::ReconcileRequired;return {false,"purchase_reconcile"};}
        if(!accepted){state_=State::Idle;candidate_.reset();return {false,"native_rejected"};}
        state_=State::AwaitingSettlement;return {true,{}};
    }
    Result settled(Money observed) {
        if(state_!=State::AwaitingSettlement)return {false,"purchase_state"};
        auto result=current();
        if(!result||!same(observed,after_)){state_=State::ReconcileRequired;return {false,"settlement_reconcile"};}
        state_=State::Complete;return {true,{}};
    }
    Result cancel() {
        if(state_!=State::Prepared)return {false,"purchase_state"};
        state_=State::Idle;candidate_.reset();return {true,{}};
    }
    State state()const{return state_;}
    bool can_save()const{return state_==State::Idle||state_==State::Complete;}
    Money settlement()const{return after_;}
    const Model* committed_model()const{return state_==State::Complete?&*candidate_:nullptr;}
private:
    static bool same(Money a,Money b){return a.actor==b.actor&&a.seller==b.seller;}
    Result current()const {
        if(carried_.session()!=session_||carried_.revision()!=revision_)return {false,"stale_layout"};
        for(const auto& handle:handles_)if(registry_.resolve(handle)!=handle.item)return {false,"stale_entity"};
        return {true,{}};
    }
    const Model& carried_;const EntityRegistry& registry_;
    State state_=State::Idle;Money before_,after_;
    std::uint64_t revision_=0,session_=0;
    std::uint16_t actor_=0xffff,seller_=0xffff;
    std::vector<EntityRegistry::Handle> handles_;
    std::map<Id,std::uint16_t> items_;
    std::optional<Model> candidate_;
};
}
