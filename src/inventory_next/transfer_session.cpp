#include "transfer_session.h"
#include <limits>
namespace inventory_next {
void TransferSession::clear() { state_=State::Idle;candidate_.reset();subtree_.clear();dispatch_={}; }
Result TransferSession::current() {
    if(model_.session()!=ticket_.command.session||model_.revision()!=ticket_.command.revision)return {false,"stale_model"};
    if(!registry_.resolve(dispatch_.source)||!registry_.resolve(dispatch_.destination))return {false,"stale_entity"};
    for(const auto& handle:subtree_)if(!registry_.resolve(handle))return {false,"stale_entity"};
    return access_.preview(ticket_).result;
}
Result TransferSession::prepare(const AccessGate::Ticket& ticket) {
    if(state_!=State::Idle)return {false,"transfer_busy"};
    const auto& command=ticket.command;
    if(command.operation!=Operation::Move&&command.operation!=Operation::Equip&&command.operation!=Operation::Mount)return {false,"native_operation"};
    auto admitted=access_.preview(ticket).result;if(!admitted)return admitted;
    const auto& item=model_.nodes().at(command.item);
    auto source=registry_.handle(item.parent),target=registry_.handle(command.destination),handle=registry_.handle(command.item);
    if(!source||!target||!handle)return {false,"offline_entity"};
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
    // Publish prepared state only after allocations and validation finish.
    candidate_=std::move(candidate);subtree_=std::move(tree);ticket_=ticket;
    dispatch_={++next_token_,*handle,*source,*target,command};state_=State::Prepared;
    return {true,{}};
}
Result TransferSession::dispatch(Dispatch& output) {
    if(state_!=State::Prepared)return {false,"transfer_state"};
    auto result=current();if(!result){clear();return result;}
    output=dispatch_;state_=State::Dispatched;return {true,{}};
}
Result TransferSession::acknowledge(std::uint64_t token,std::uint16_t observed_parent,bool accepted) {
    if(state_!=State::Dispatched||token!=dispatch_.token)return {false,"transfer_ack"};
    auto result=current();
    if(!result){state_=State::ReconcileRequired;return result;}
    const auto expected=accepted?dispatch_.destination.live:dispatch_.source.live;
    if(observed_parent!=expected){state_=State::ReconcileRequired;return {false,"native_parent_mismatch"};}
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
