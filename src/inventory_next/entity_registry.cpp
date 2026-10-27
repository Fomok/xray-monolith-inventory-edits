#include "entity_registry.h"
#include <atomic>
#include <limits>
#include <stdexcept>

namespace inventory_next {
namespace {
std::atomic<std::uint64_t> epochs{0};
std::uint64_t next_epoch() {
    auto previous = epochs.load();
    do {
        if (previous == std::numeric_limits<std::uint64_t>::max())
            throw std::overflow_error("entity registry epoch exhausted");
    } while (!epochs.compare_exchange_weak(previous, previous + 1));
    return previous + 1;
}
}
EntityRegistry::EntityRegistry() : epoch_(next_epoch()) {}
Result EntityRegistry::restore_known(const std::vector<Id>& ids) {
    if (ids.size() > 4096) return {false,"entity_count"};
    std::set<Id> candidate;
    for (auto id : ids) if (!id || !candidate.insert(id).second) return {false,"entity_id"};
    // Allocate epoch before publishing to preserve current bindings on failure.
    const auto epoch = next_epoch();
    known_.swap(candidate); online_.clear(); slots_.clear();
    epoch_ = epoch; generation_ = 0;
    return {true,{}};
}
Result EntityRegistry::add(Id id) {
    if (!id) return {false,"entity_id"};
    if (known(id)) return {true,{}};
    if (known_.size() >= 4096) return {false,"entity_count"};
    known_.insert(id); return {true,{}};
}
Result EntityRegistry::bind(Id id, std::uint16_t live) {
    if (!known(id)) return {false,"unknown_entity"};
    if (live == 0xffff) return {false,"invalid_live_id"};
    const auto existing = online_.find(id);
    if (existing != online_.end())
        return existing->second.live == live ? Result{true,{}} : Result{false,"already_online"};
    if (slots_.count(live)) return {false,"live_id_in_use"};
    if (generation_ == std::numeric_limits<std::uint64_t>::max()) return {false,"generation_exhausted"};
    const Handle next{id,live,epoch_,generation_ + 1};
    // Roll back the first allocation if the second allocation fails.
    online_.emplace(id,next);
    try { slots_.emplace(live,id); } catch (...) { online_.erase(id); throw; }
    ++generation_;
    return {true,{}};
}
Result EntityRegistry::forget(Id id) {
    if (online_.count(id)) return {false,"still_online"};
    if (!known_.erase(id)) return {false,"unknown_entity"};
    return {true,{}};
}
Id EntityRegistry::resolve(const Handle& h) const {
    if (h.epoch != epoch_ || !h.generation) return 0;
    const auto found = online_.find(h.item);
    if (found == online_.end()) return 0;
    const auto& live = found->second;
    return live.live == h.live && live.generation == h.generation ? h.item : 0;
}
Result EntityRegistry::unbind(const Handle& h) {
    if (!resolve(h)) return {false,"stale_entity"};
    slots_.erase(h.live); online_.erase(h.item);
    return {true,{}};
}
std::optional<EntityRegistry::Handle> EntityRegistry::handle(Id id) const {
    const auto found = online_.find(id);
    return found == online_.end() ? std::nullopt : std::optional<Handle>{found->second};
}
} // namespace inventory_next
