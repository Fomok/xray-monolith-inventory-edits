#pragma once
#include <atomic>
#include <cstdint>
#include <vector>

namespace inventory_rig_transfer
{
using Id = std::uint16_t;
using Token = std::uint64_t;
constexpr Id none = 0xffff;
inline Token next_identity()
{
    static std::atomic<Token> next{1};
    return next.fetch_add(1, std::memory_order_relaxed);
}
struct Entry
{
    Id item, from, to, rig;
    Token item_token, rig_token;
    std::uint32_t started;
    bool warned = false;
};
struct Observation
{
    bool endpoints_live = false;
    Token item_token = 0, rig_token = 0;
    Id parent = none;
};
enum class Request { refused, arrived, pending, queued };

// One registry per actor inventory. It holds no object pointers and never
// persists network events. Lifetimes distinguish recycled IDs after destruction.
class Registry
{
public:
    const std::vector<Entry>& entries() const { return pending_; }
    bool pending(Id id) const
    {
        for (const auto& e : pending_) if (e.item == id) return true;
        return false;
    }
    bool rig_pending(Id id) const
    {
        for (const auto& e : pending_) if (e.rig == id) return true;
        return false;
    }
    template<class Lookup, class Warn> void settle(Lookup lookup, Warn warn, std::uint32_t now)
    {
        for (auto it = pending_.begin(); it != pending_.end();)
        {
            const auto state = lookup(*it);
            const bool live = state.endpoints_live && state.item_token == it->item_token
                && state.rig_token == it->rig_token;
            if (!live || state.parent == it->to ||
                (state.parent != none && state.parent != it->from))
            {
                if (live && state.parent == it->to) finished_.push_back(*it);
                it = pending_.erase(it);
            }
            else
            {
                // A detached object is an intermediate SELL/BUY state, not
                // failure. Never cancel/reissue merely because time elapsed.
                if (!it->warned && std::uint32_t(now - it->started) > 10000)
                { it->warned = true; warn(*it); }
                ++it;
            }
        }
    }
    Request request(const Entry& e, const Observation& state)
    {
        if (e.item == none || e.from == none || e.to == none || e.rig == none ||
            e.from == e.to || e.item == e.rig || !e.item_token || !e.rig_token ||
            !state.endpoints_live || state.item_token != e.item_token || state.rig_token != e.rig_token)
            return Request::refused;
        for (const auto& old : pending_)
            if (old.item == e.item)
                return old.to == e.to && old.from == e.from && old.rig == e.rig
                    && old.item_token == e.item_token && old.rig_token == e.rig_token
                    ? Request::pending : Request::refused;
        if (state.parent == e.to) return Request::arrived;
        if (state.parent != e.from) return Request::refused;
        pending_.push_back(e); // Record BEFORE dispatch, including synchronous callbacks.
        return Request::queued;
    }
    bool take_finished(Entry& e)
    {
        if (finished_.empty()) return false;
        e = finished_.back(); finished_.pop_back(); return true;
    }
    void forget(Id id)
    {
        auto remove = [id](std::vector<Entry>& entries)
        {
            for (auto it = entries.begin(); it != entries.end();)
                if (it->item == id || it->rig == id || it->from == id || it->to == id)
                    it = entries.erase(it);
                else ++it;
        };
        remove(pending_); remove(finished_);
    }
    void clear() { pending_.clear(); finished_.clear(); }
private:
    std::vector<Entry> pending_, finished_;
};
}
