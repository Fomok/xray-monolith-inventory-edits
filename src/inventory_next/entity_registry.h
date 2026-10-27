#pragma once
#include "model.h"
#include <optional>
#include <set>

namespace inventory_next {
// A live u16 object ID is not persistent identity: the engine reuses these slots.
// The adapter restores core IDs from save records, then binds each online object.
class EntityRegistry {
public:
    struct Handle {
        Id item = 0;
        std::uint16_t live = 0xffff;
        std::uint64_t epoch = 0, generation = 0;
    };
    EntityRegistry();
    EntityRegistry(const EntityRegistry&) = delete;
    EntityRegistry& operator=(const EntityRegistry&) = delete;
    Result restore_known(const std::vector<Id>& persistent_ids);
    Result add(Id persistent_id);
    Result forget(Id persistent_id); // destroyed core entity, never merely offline
    Result bind(Id persistent_id, std::uint16_t live_id);
    Result unbind(const Handle& handle);
    std::optional<Handle> handle(Id persistent_id) const;
    Id resolve(const Handle& handle) const;
    bool known(Id persistent_id) const { return known_.count(persistent_id) != 0; }
    std::size_t online_count() const { return online_.size(); }
    std::uint64_t epoch() const { return epoch_; }
private:
    std::set<Id> known_;
    std::map<Id, Handle> online_;
    std::map<std::uint16_t, Id> slots_;
    std::uint64_t epoch_ = 0, generation_ = 0;
};
} // namespace inventory_next
