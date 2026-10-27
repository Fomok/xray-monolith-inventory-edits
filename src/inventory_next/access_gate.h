#pragma once
#include "model.h"
#include <map>

namespace inventory_next {
// Tested boundary for observations supplied by a future engine adapter. This class
// does not discover world entities, perform LOS checks, or deliver engine events.
// Single-threaded: observations and core commit share the same command thread.
class AccessGate {
public:
    struct Observation { bool exists = false, reachable = false; };
    struct ItemObservation { bool revealed = false, actionable = false; };
    struct Ticket { Command command; std::uint64_t access_revision = 0; };
    explicit AccessGate(Model& model) : model_(model), session_(model.session()) {}
    Result observe(Id world_root, bool exists, bool reachable) {
        refresh();
        auto n = model_.nodes().find(world_root);
        if (n == model_.nodes().end() || n->second.kind != Kind::WorldRoot) return {false,"world_root"};
        auto previous = observations_.find(world_root);
        if (previous == observations_.end() || previous->second.exists != exists || previous->second.reachable != reachable) {
            observations_[world_root] = {exists, reachable}; ++revision_;
        }
        return {true,{}};
    }
    std::uint64_t revision() { refresh(); return revision_; }
    Result observe_item(Id item, bool revealed, bool actionable) {
        refresh();
        auto n=model_.nodes().find(item);
        if(n==model_.nodes().end()||n->second.kind!=Kind::Item) return {false,"item"};
        auto previous=items_.find(item);
        if(previous==items_.end()||previous->second.revealed!=revealed||previous->second.actionable!=actionable) {
            items_[item]={revealed,actionable};++revision_;
        }
        return {true,{}};
    }
    Result execute(const Ticket& ticket) {
        auto r = check(ticket); return r ? model_.execute(ticket.command) : r;
    }
    Preview preview(const Ticket& ticket) {
        auto r = check(ticket);
        if (!r) return {r,model_.carried_mass(),model_.nodes().size(),model_.revision()};
        return model_.preview(ticket.command);
    }
    std::vector<Id> children(Id container) {
        refresh(); std::vector<Id> result;
        if (!access(container,false)) return result;
        for (const auto& [id,n] : model_.nodes()) if (n.kind==Kind::Item && n.parent==container && access(id,false)) result.push_back(id);
        return result;
    }
    std::vector<Id> visible_objects() {
        refresh();std::vector<Id> result;
        for(const auto& [id,n]:model_.nodes()) { (void)n; if(access(id,false)) result.push_back(id); }
        return result;
    }
private:
    void refresh() {
        if (session_ != model_.session()) {
            observations_.clear(); items_.clear(); session_ = model_.session(); ++revision_;
        }
    }
    Result access(Id id, bool action=true) const {
        std::vector<Id> lineage;
        for (unsigned depth=0;depth<=64;++depth) {
            auto node=model_.nodes().find(id);
            if (node==model_.nodes().end()) return {false,"item"};
            if (node->second.kind==Kind::ActorRoot) return {true,{}};
            if (node->second.kind==Kind::WorldRoot) {
                auto observed=observations_.find(id);
                if (observed==observations_.end()) return {false,"unobserved_source"};
                if (!observed->second.exists || !observed->second.reachable) return {false,"world_unavailable"};
                for(Id item:lineage) {
                    auto policy=items_.find(item);
                    if(policy==items_.end()||!policy->second.revealed) return {false,"unrevealed_item"};
                    if(action&&!policy->second.actionable) return {false,"item_action_denied"};
                }
                return {true,{}};
            }
            lineage.push_back(id);
            id=node->second.parent;
        }
        return {false,"depth"};
    }
    Result check(const Ticket& ticket) {
        refresh();
        if (ticket.access_revision!=revision_) return {false,"stale_access"};
        const auto& c=ticket.command;
        auto r=access(c.operation==Operation::Spawn ? c.spawned.parent : c.item);
        if (!r) return r;
        if (c.operation==Operation::Move || c.operation==Operation::Split ||
            c.operation==Operation::Merge || c.operation==Operation::Equip || c.operation==Operation::Mount) return access(c.destination);
        return {true,{}};
    }
    Model& model_;
    std::uint64_t session_, revision_=0;
    std::map<Id,Observation> observations_;
    std::map<Id,ItemObservation> items_;
};
}
