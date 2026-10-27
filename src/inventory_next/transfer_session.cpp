#include "transfer_session.h"
#include <limits>
namespace inventory_next {
void TransferSession::clear() { state_=State::Idle;candidate_.reset();subtree_.clear();physical_ancestry_.clear();dispatch_={};logical_source_.reset();logical_destination_.reset(); }
Result TransferSession::current() {
    if(model_.session()!=ticket_.command.session||model_.revision()!=ticket_.command.revision)return {false,"stale_model"};
    if(!registry_.resolve(dispatch_.destination)||(dispatch_.source&&!registry_.resolve(*dispatch_.source)))return {false,"stale_entity"};
    if((logical_source_&&!registry_.resolve(*logical_source_))||
       (logical_destination_&&!registry_.resolve(*logical_destination_)))return {false,"stale_entity"};
    if(!dispatch_.source) {
        const auto parent=model_.nodes().at(ticket_.command.item).parent;
        if(model_.nodes().at(parent).kind!=Kind::WorldRoot||registry_.known(parent))return {false,"native_ground_source"};
    }
    for(const auto& handle:subtree_)if(!registry_.resolve(handle))return {false,"stale_entity"};
    for(const auto& handle:physical_ancestry_)if(!registry_.resolve(handle))return {false,"stale_entity"};
    return access_.preview(ticket_).result;
}
Result TransferSession::prepare(const AccessGate::Ticket& ticket,bool ground_source) {
    return prepare(ticket,ground_source,PhysicalOwners{});
}
Result TransferSession::prepare(const AccessGate::Ticket& ticket,bool ground_source,const PhysicalOwners& physical) {
    if(state_!=State::Idle)return {false,"transfer_busy"};
    const auto& command=ticket.command;
    if(command.operation!=Operation::Move&&command.operation!=Operation::Equip&&command.operation!=Operation::Mount)return {false,"native_operation"};
    auto admitted=access_.preview(ticket).result;if(!admitted)return admitted;
    const auto& item=model_.nodes().at(command.item);
    auto source=registry_.handle(item.parent),target=registry_.handle(command.destination),handle=registry_.handle(command.item);
    if(!target||!handle)return {false,"offline_entity"};
    const auto logical_source=source,logical_target=target;
    std::vector<EntityRegistry::Handle> physical_ancestry;
    auto physical_owner=[&](const EntityRegistry::Handle& actual,Id logical,bool flattened) {
        if(!registry_.resolve(actual))return false;
        const auto root=model_.nodes().find(actual.item);
        if(root==model_.nodes().end()||(root->second.kind!=Kind::ActorRoot&&
           !(flattened&&root->second.kind==Kind::WorldRoot)))return false;
        for(unsigned depth=0;logical&&depth<=64;++depth) {
            auto ancestor=registry_.handle(logical);if(!ancestor)return false;
            physical_ancestry.push_back(*ancestor);
            if(logical==actual.item)return true;
            logical=model_.nodes().at(logical).parent;
        }
        return false;
    };
    if(physical.destination) {
        if(!physical_owner(*physical.destination,command.destination,physical.flatten_destination))return {false,"native_physical_owner"};
        target=physical.destination;
    }
    if(ground_source) {
        if(physical.source||model_.nodes().at(item.parent).kind!=Kind::WorldRoot||registry_.known(item.parent))return {false,"native_ground_source"};
        source.reset();
    } else if(!source)return {false,"offline_entity"};
    if(physical.source) {
        if(!physical_owner(*physical.source,item.parent,physical.flatten_source))return {false,"native_physical_owner"};
        source=physical.source;
    }
    auto custody_root=[&](const std::optional<EntityRegistry::Handle>& owner) {
        if(!owner)return false;
        const auto found=model_.nodes().find(owner->item);
        return found!=model_.nodes().end()&&found->second.kind!=Kind::Item;
    };
    if((physical.flatten_source&&!custody_root(source))||
       (physical.flatten_destination&&!custody_root(target)))return {false,"native_subtree_owner"};
    if(next_token_==std::numeric_limits<std::uint64_t>::max())return {false,"transfer_tokens"};
    std::vector<EntityRegistry::Handle> tree;
    for(const auto& pair:model_.nodes()) {
        Id id=pair.first;
        for(unsigned depth=0;id&&depth<=64;++depth){
            if(id==command.item){auto h=registry_.handle(pair.first);if(!h)return {false,"offline_entity"};tree.push_back(*h);break;}
            id=model_.nodes().at(id).parent;
        }
    }
    Model candidate=model_;auto result=candidate.execute(command);if(!result)return result;
    std::vector<Ownership> ownership;
    for(const auto& member:tree) {
        const bool shell=member.item==command.item;
        const auto parent=registry_.handle(model_.nodes().at(member.item).parent);
        ownership.push_back({member,(shell||physical.flatten_source)?source:parent,
                                    (shell||physical.flatten_destination)?target:parent});
        if(!shell&&(!ownership.back().source||!ownership.back().destination))return {false,"offline_entity"};
    }
    // Publish prepared state only after allocations and validation finish.
    candidate_=std::move(candidate);subtree_=std::move(tree);ticket_=ticket;
    physical_ancestry_=std::move(physical_ancestry);
    logical_source_=ground_source?std::nullopt:logical_source;logical_destination_=logical_target;
    dispatch_={++next_token_,*handle,*target,source,command,std::move(ownership)};state_=State::Prepared;
    return {true,{}};
}
Result TransferSession::dispatch(Dispatch& output) {
    if(state_!=State::Prepared)return {false,"transfer_state"};
    auto result=current();if(!result){clear();return result;}
    output=dispatch_;state_=State::Dispatched;return {true,{}};
}
Result TransferSession::acknowledge(std::uint64_t token,std::uint16_t observed_parent,bool accepted) {
    if(state_!=State::Dispatched||token!=dispatch_.token)return {false,"transfer_ack"};
    if(dispatch_.subtree.size()!=1)return {false,"subtree_ack_required"};
    return acknowledge_subtree(token,{{dispatch_.item,observed_parent}},accepted);
}
Result TransferSession::acknowledge_subtree(std::uint64_t token,const std::vector<ParentObservation>& observed,bool accepted) {
    if(state_!=State::Dispatched||token!=dispatch_.token)return {false,"transfer_ack"};
    auto result=current();
    if(!result){state_=State::ReconcileRequired;return result;}
    if(observed.size()!=dispatch_.subtree.size()) {
        state_=State::ReconcileRequired;return {false,"native_subtree_incomplete"};
    }
    std::map<Id,std::uint16_t> parents;
    for(const auto& observation:observed) {
        if(!registry_.resolve(observation.item)||!parents.emplace(observation.item.item,observation.parent).second) {
            state_=State::ReconcileRequired;return {false,"native_subtree_identity"};
        }
    }
    for(const auto& member:dispatch_.subtree) {
        const auto found=parents.find(member.item.item);
        const auto& owner=accepted?member.destination:member.source;
        const auto expected=owner?owner->live:std::uint16_t{0xffff};
        if(found==parents.end()||found->second!=expected) {
            state_=State::ReconcileRequired;return {false,"native_parent_mismatch"};
        }
    }
    if(!accepted){clear();return {false,"native_rejected"};}
    model_=std::move(*candidate_);clear();return {true,{}};
}
Result TransferSession::cancel() {
    if(state_==State::Dispatched||state_==State::ReconcileRequired)return {false,"native_reconcile_required"};
    clear();return {true,{}};
}
Result TransferSession::reconciled() {
    if(state_!=State::ReconcileRequired)return {false,"transfer_state"};
    if(model_.session()==ticket_.command.session)return {false,"rebuild_required"};
    clear();return {true,{}};
}
}
