#include "identity_ledger.h"
#include <limits>
#include <set>
#include <sstream>
#include <locale>
namespace inventory_next {
Id IdentityLedger::lookup(std::uint16_t server) const {
    const auto found=entries_.find(server);return found==entries_.end()?0:found->second;
}
Result IdentityLedger::enroll(std::uint16_t server,Id& core) {
    if(server==0xffff)return {false,"invalid_server_id"};
    if(auto found=lookup(server)){core=found;return {true,{}};}
    if(entries_.size()>=4096)return {false,"identity_capacity"};
    if(high_water_==std::numeric_limits<Id>::max())return {false,"identity_exhausted"};
    const Id next=high_water_+1;entries_.emplace(server,next);high_water_=next;core=next;return {true,{}};
}
Result IdentityLedger::remap(std::uint16_t before,std::uint16_t after) {
    if(before==after||!lookup(before))return {true,{}};
    if(after==0xffff||lookup(after))return {false,"identity_remap"};
    entries_.emplace(after,lookup(before));entries_.erase(before);return {true,{}};
}
std::string IdentityLedger::serialize() const {
    std::ostringstream out;out.imbue(std::locale::classic());
    out<<"SQA3_IDENTITIES 1 "<<high_water_<<' '<<entries_.size()<<'\n';
    for(const auto& [server,core]:entries_)out<<server<<' '<<core<<'\n';
    return out.str();
}
Result IdentityLedger::deserialize(const std::string& bytes) {
    if(bytes.size()>131072)return {false,"identity_size"};
    std::istringstream in(bytes);in.imbue(std::locale::classic());
    std::string magic;unsigned version;Id high;std::size_t count;
    if(!(in>>magic>>version>>high>>count)||magic!="SQA3_IDENTITIES"||version!=1||high<99||count>4096)return {false,"identity_header"};
    IdentityLedger candidate;candidate.high_water_=high;std::set<Id> unique;
    for(std::size_t i=0;i<count;++i){unsigned server;Id core;
        if(!(in>>server>>core)||server>=0xffff||core<100||core>high||!unique.insert(core).second||
            !candidate.entries_.emplace(static_cast<std::uint16_t>(server),core).second)return {false,"identity_record"};
    }
    in>>std::ws;if(!in.eof())return {false,"identity_trailing"};
    // Canonical encoding rejects negative/overflow/noncanonical numeric spellings.
    if(candidate.serialize()!=bytes)return {false,"identity_encoding"};
    *this=std::move(candidate);return {true,{}};
}
}
