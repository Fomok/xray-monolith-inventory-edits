#pragma once
#include "model.h"
namespace inventory_next {
// Persistent ALife IDs, not online handles. Registry handles remain process-local.
class IdentityLedger {
public:
    Result enroll(std::uint16_t server, Id& core);
    Result remap(std::uint16_t before, std::uint16_t after);
    void retire(std::uint16_t server) { entries_.erase(server); }
    Id lookup(std::uint16_t server) const;
    const std::map<std::uint16_t,Id>& entries() const { return entries_; }
    Id high_water() const { return high_water_; }
    std::string serialize() const;
    Result deserialize(const std::string& bytes);
private:
    std::map<std::uint16_t,Id> entries_;
    Id high_water_ = 99; // roots reserved below100
};
}
