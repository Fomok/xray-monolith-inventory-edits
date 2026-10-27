#pragma once
#include "identity_ledger.h"
#include "entity_registry.h"
namespace inventory_next {
// Exact online/offline/remap callback state used by the native adapter.
// Lifetime tokens are opaque integers, never dereferenced.
class IdentityState {
public:
    const IdentityLedger& ledger() const { return ledger_; }
    const EntityRegistry& registry() const { return registry_; }
    Result enroll(std::uint16_t server,Id& core) { return ledger_.enroll(server,core); }
    Result restore(const std::string& bytes) {
        IdentityLedger next;auto result=next.deserialize(bytes);if(!result)return result;
        std::vector<Id> ids;for(const auto& entry:next.entries())ids.push_back(entry.second);
        result=registry_.restore_known(ids);if(!result)return result;
        ledger_=std::move(next);objects_.clear();return {true,{}};
    }
    Result reset() { return restore(IdentityLedger{}.serialize()); }
    Result online(std::uint16_t server,std::uintptr_t token) {
        if(!token)return {false,"lifetime_token"};
        const auto core=ledger_.lookup(server);if(!core)return {true,{}};
        const auto found=objects_.find(server);
        if(found!=objects_.end()&&found->second!=token)return {false,"live_reuse"};
        auto added=registry_.add(core);if(!added)return added;
        const bool inserted=objects_.emplace(server,token).second;
        try { auto bound=registry_.bind(core,server);if(!bound&&inserted)objects_.erase(server);return bound; }
        catch (...) { if(inserted)objects_.erase(server);throw; }
    }
    Result offline(std::uint16_t server,std::uintptr_t token) {
        auto found=objects_.find(server);if(found==objects_.end()||found->second!=token)return {true,{}};
        if(auto handle=registry_.handle(ledger_.lookup(server))){auto r=registry_.unbind(*handle);if(!r)return r;}
        objects_.erase(found);return {true,{}};
    }
    Result remap(std::uint16_t before,std::uint16_t after) {
        if(before==after||!ledger_.lookup(before))return {true,{}};
        IdentityLedger next=ledger_;auto result=next.remap(before,after);if(!result)return result;
        if(auto handle=registry_.handle(ledger_.lookup(before))){result=registry_.unbind(*handle);if(!result)return result;}
        objects_.erase(before);ledger_=std::move(next);return {true,{}};
    }
    Result retire(std::uint16_t server) {
        const auto core=ledger_.lookup(server);
        if(auto handle=registry_.handle(core)){auto r=registry_.unbind(*handle);if(!r)return r;}
        if(registry_.known(core)){auto r=registry_.forget(core);if(!r)return r;}
        objects_.erase(server);ledger_.retire(server);return {true,{}};
    }
private:
    IdentityLedger ledger_;
    EntityRegistry registry_;
    std::map<std::uint16_t,std::uintptr_t> objects_;
};
}
