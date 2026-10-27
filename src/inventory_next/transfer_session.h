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
    struct Ownership {
        EntityRegistry::Handle item;
        std::optional<EntityRegistry::Handle> source, destination;
    };
    struct ParentObservation {
        EntityRegistry::Handle item;
        std::uint16_t parent=0xffff;
    };
    struct Dispatch {
        std::uint64_t token=0;
        EntityRegistry::Handle item, destination;
        std::optional<EntityRegistry::Handle> source; // absent only for explicit unowned ground
        Command command;
        std::vector<Ownership> subtree;
    };
    struct PhysicalOwners {
        std::optional<EntityRegistry::Handle> source, destination;
        // Explicit native custody policy. Logical children can all be held by
        // an actor or world inventory root while retaining their logical tree.
        bool flatten_source=false, flatten_destination=false;
    };
    TransferSession(Model& model,AccessGate& access,const EntityRegistry& registry)
        : model_(model),access_(access),registry_(registry) {}
    // Ground has no native parent entity. The adapter must verify parent=0xffff
    // before prepare AND dispatch; the logical world root must be unregistered.
    // Logical storage can be backed by physical actor ownership. Overrides must
    // resolve to the corresponding logical endpoint's actual actor ancestor.
    Result prepare(const AccessGate::Ticket& ticket,bool ground_source=false);
    Result prepare(const AccessGate::Ticket& ticket,bool ground_source,const PhysicalOwners& physical);
    Result dispatch(Dispatch& output);
    Result acknowledge(std::uint64_t token,std::uint16_t observed_parent,bool accepted);
    Result acknowledge_subtree(std::uint64_t token,const std::vector<ParentObservation>& observed,bool accepted);
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
    std::optional<EntityRegistry::Handle> logical_source_,logical_destination_;
    std::vector<EntityRegistry::Handle> subtree_;
    std::vector<EntityRegistry::Handle> physical_ancestry_;
    std::optional<Model> candidate_;
    std::uint64_t next_token_=0;
};
}
