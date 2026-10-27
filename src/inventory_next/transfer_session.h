#pragma once
#include "access_gate.h"
#include "entity_registry.h"
#include <optional>
namespace inventory_next {
// Native adapter delivers events only after dispatch(), then acknowledges the
// observed native parent. Never interprets event-send success as ownership success.
// Keep one coordinator per ownership authority and share it across all views.
// Independent instances do not share reservations; UI panels must not own one each.
class TransferSession {
public:
    enum class State { Idle, Prepared, Dispatched, ReconcileRequired };
    struct Dispatch {
        std::uint64_t token=0;
        EntityRegistry::Handle item, destination;
        std::optional<EntityRegistry::Handle> source; // absent only for explicit unowned ground
        Command command;
    };
    TransferSession(Model& model,AccessGate& access,const EntityRegistry& registry)
        : model_(model),access_(access),registry_(registry) {}
    // Ground has no native parent entity. The adapter must verify parent=0xffff
    // before prepare AND dispatch; the logical world root must be unregistered.
    Result prepare(const AccessGate::Ticket& ticket,bool ground_source=false);
    Result dispatch(Dispatch& output);
    Result acknowledge(std::uint64_t token,std::uint16_t observed_parent,bool accepted);
    Result cancel();
    Result reconciled(); // caller must rebuild Model from native truth first
    State state() const { return state_; }
    bool can_save() const { return state_==State::Idle; }
private:
    Result current();
    void clear();
    Model& model_;
    AccessGate& access_;
    const EntityRegistry& registry_;
    State state_=State::Idle;
    AccessGate::Ticket ticket_;
    Dispatch dispatch_;
    std::vector<EntityRegistry::Handle> subtree_;
    std::optional<Model> candidate_;
    std::uint64_t next_token_=0;
};
}
