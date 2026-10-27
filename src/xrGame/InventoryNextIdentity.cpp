#define SQA3_EXCEPTION_BOUNDARY
#include "stdafx.h"
#include "InventoryNextIdentity.h"
#include "InventoryNextProbe.h"
#include "../inventory_next/identity_ledger.h"
#include "../inventory_next/identity_state.h"
#include "inventory_item.h"
#include "PhysicsShellHolder.h"
#include "level.h"
#include "ai_space.h"
#include "alife_simulator.h"
#include "alife_object_registry.h"
#include <exception>
namespace {
constexpr u32 identity_chunk = 0x33514944; // SQA3 identity chunk, outside legacy IDs
inventory_next::IdentityState& identities() { static inventory_next::IdentityState value;return value; }
const inventory_next::IdentityLedger& ledger() { return identities().ledger(); }
}
bool InventoryNextEnabled() noexcept try { return strstr(Core.Params,"-inventory-lab") && strstr(Core.Params,"-dbg"); } catch (...) { CHECK_OR_EXIT(false,"SQA3 identity boundary failure"); return 0; }
u64 InventoryNextIdentity(u16 server_id) noexcept try {
    if(!InventoryNextEnabled()||!ai().get_alife()||!ai().alife().objects().object(server_id,true))return 0;
    u16 parent=server_id;
    for(unsigned depth=0;;++depth) {
        const auto object=ai().alife().objects().object(parent,true);
        if(depth>64||!object||!object->can_save()||object->redundant()) {
            Msg("! SQA3 identity enrollment refused for unsaveable entity/ancestor %u",server_id);return 0;
        }
        if(object->ID_Parent==0xffff)break;parent=object->ID_Parent;
    }
    try {
        inventory_next::Id id=0;const auto result=identities().enroll(server_id,id);
        if(!result) { Msg("! SQA3 identity: %s",result.error.c_str());return 0; }
        return id;
    } catch(const std::exception& e) { Msg("! SQA3 identity: %s",e.what());return 0; }
} catch (...) { CHECK_OR_EXIT(false,"SQA3 identity boundary failure"); return 0; }
void InventoryNextIdentityReset() noexcept try {
    InventoryNextLayoutReset();
    const auto result=identities().reset();CHECK_OR_EXIT(result.ok,"SQA3 identity reset failed");
} catch (...) { CHECK_OR_EXIT(false,"SQA3 identity boundary failure"); }
void InventoryNextIdentityRetire(u16 server_id) noexcept try {
    if(!InventoryNextEnabled())return;
    const auto result=identities().retire(server_id);CHECK_OR_EXIT(result.ok,"SQA3 identity retirement failed");
} catch (...) { CHECK_OR_EXIT(false,"SQA3 identity boundary failure"); }
void InventoryNextIdentityRemap(u16 before,u16 after) noexcept try {
    if(!InventoryNextEnabled())return;
    const auto result=identities().remap(before,after);CHECK_OR_EXIT(result.ok,"SQA3 identity remap conflict");
} catch (...) { CHECK_OR_EXIT(false,"SQA3 identity boundary failure"); }
bool InventoryNextIdentityCanSave() noexcept try {
    if(!InventoryNextEnabled())return true;
    if(!ai().get_alife())return ledger().entries().empty();
    if(!InventoryNextLayoutSync())return false;
    for(const auto& entry:ledger().entries()) {
        u16 id=entry.first;
        for(unsigned depth=0;;++depth) {
            const auto object=ai().alife().objects().object(id,true);
            if(depth>64||!object||!object->can_save()||object->redundant()) {
                Msg("! SQA3 save refused: enrolled entity %u or its parent is not saveable",entry.first);
                return false;
            }
            if(object->ID_Parent==0xffff)break;
            id=object->ID_Parent;
        }
    }
    return true;
} catch (...) { Msg("! SQA3 identity save validation failed");return false; }
void InventoryNextIdentitySave(IWriter& stream) noexcept try {
    if(!InventoryNextEnabled())return;
    // Save even an empty map: high-water survives destruction of all enrolled items.
    const auto bytes=ledger().serialize();
    stream.open_chunk(identity_chunk);stream.w(bytes.data(),static_cast<u32>(bytes.size()));stream.close_chunk();
    InventoryNextLayoutSave(stream);
} catch (...) { CHECK_OR_EXIT(false,"SQA3 identity boundary failure"); }
void InventoryNextIdentityLoad(IReader& stream) noexcept try {
    IReader* chunk=stream.open_chunk(identity_chunk);
    if(!chunk) { InventoryNextIdentityReset();return; }
    CHECK_OR_EXIT(InventoryNextEnabled(),"This SQA3 development save requires -inventory-lab -dbg");
    CHECK_OR_EXIT(chunk->length()<=131072,"Invalid SQA3 identity chunk size");
    const std::string bytes(reinterpret_cast<const char*>(chunk->pointer()),chunk->length());
    const auto result=identities().restore(bytes);chunk->close();
    CHECK_OR_EXIT(result.ok,"Invalid SQA3 persistent identity chunk");
    InventoryNextLayoutLoad(stream);
} catch (...) { CHECK_OR_EXIT(false,"SQA3 identity boundary failure"); }
void InventoryNextIdentityValidate() noexcept try {
    if(!InventoryNextEnabled())return;
    for(const auto& entry:ledger().entries())
        CHECK_OR_EXIT(ai().alife().objects().object(entry.first,true)!=nullptr,"SQA3 saved identity refers to a missing ALife entity");
} catch (...) { CHECK_OR_EXIT(false,"SQA3 identity boundary failure"); }

const inventory_next::EntityRegistry& InventoryNextLiveRegistry() noexcept try { return identities().registry(); }
catch (...) { CHECK_OR_EXIT(false,"SQA3 live registry initialization failed");std::terminate(); }
void InventoryNextIdentityOnline(CInventoryItem& item) noexcept try {
    if(!InventoryNextEnabled())return;
    const auto result=identities().online(item.object().ID(),reinterpret_cast<std::uintptr_t>(&item));
    CHECK_OR_EXIT(result.ok,"SQA3 live identity binding failed");
} catch (...) { CHECK_OR_EXIT(false,"SQA3 online identity boundary failure"); }
void InventoryNextIdentityOffline(CInventoryItem& item) noexcept try {
    if(!InventoryNextEnabled())return;
    const auto result=identities().offline(item.object().ID(),reinterpret_cast<std::uintptr_t>(&item));
    CHECK_OR_EXIT(result.ok,"SQA3 offline identity unbind failed");
} catch (...) { CHECK_OR_EXIT(false,"SQA3 offline identity boundary failure"); }
