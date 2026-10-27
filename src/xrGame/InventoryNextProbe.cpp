#define SQA3_EXCEPTION_BOUNDARY
#include "stdafx.h"
#include "InventoryNextProbe.h"
#include "InventoryNextIdentity.h"
#include "actor.h"
#include "inventory.h"
#include "inventory_item.h"
#include "level.h"
#include "UIGameCustom.h"
#include "ui/UIActorMenu.h"
#include "ui/InventoryNextView.h"
#include "ui/UITextureMaster.h"
#include "xr_level_controller.h"
#include "../xrEngine/xr_input.h"
#include "../inventory_next/native_layout.h"
#include "../inventory_next/entity_registry.h"
#include "../inventory_next/persistence.h"
#include <cmath>
#include <exception>
#include <stdexcept>

namespace {
inventory_next::ActorLayout& layout(){static inventory_next::ActorLayout value;return value;}
constexpr u32 layout_chunk=0x33514c59;

}

bool InventoryNextLayoutSync() noexcept try {
    if(!InventoryNextEnabled()||!g_pGameLevel||!Actor())return true;
    using namespace inventory_next;
    std::vector<Node> nodes;
        for (auto* item : Actor()->inventory().m_all) {
            if (!item || nodes.size()>=4094) { Msg("! SQA3 probe: invalid/count-limited inventory"); return false; }
            const double grams=static_cast<double>(item->Weight())*1000.0;
            const double condition=item->GetCondition();
            const Irect shape=item->GetInvGridRect();
            const int w=shape.x2,h=shape.y2;
            if (!std::isfinite(grams)||grams<0||grams>1e12||!std::isfinite(condition)||condition<0||condition>1||w<1||h<1||w>16||h>64) {
                Msg("! SQA3 probe: unsupported native value for live ID %u",item->object().ID()); return false;
            }
            Node n; n.id=InventoryNextIdentity(item->object().ID());
            if(!n.id) { Msg("! SQA3 probe requires registered persistent ALife items");return false; } InventoryNextIdentityOnline(*item); n.parent=1; n.intrinsic=static_cast<Mass>(std::llround(grams));
            n.empty={w,h}; n.half=n.empty; n.full=n.empty; n.position={};
            if(item->CurrPlace()==eItemPlaceSlot) {
                if(item->CurrSlot()>=48)return false;
                n.location=Location::Equipment;n.wear_slots=n.equipped_slots=std::uint64_t{1}<<item->CurrSlot();
            }
            n.condition=static_cast<unsigned>(std::llround(condition*10000.0));
            n.attributes["native_section"]=item->m_section_id.c_str();
            n.attributes["native_live_id"]=std::to_string(item->object().ID());
            n.attributes["native_place"]=std::to_string(item->CurrPlace());
            n.attributes["native_slot"]=std::to_string(item->CurrSlot());
            nodes.push_back(n);
        }
        const auto result=layout().reconcile(std::move(nodes));
        if(!result)Msg("! SQA3 native layout sync: %s",result.error.c_str());
        return result.ok;
} catch (...) {Msg("! SQA3 native layout sync exception");return false;}
void InventoryNextLayoutReset() noexcept try {layout().reset();} catch (...) {CHECK_OR_EXIT(false,"SQA3 layout reset failed");}
void InventoryNextLayoutSave(IWriter& stream) noexcept try {
    if(!layout().initialized())return;
    const auto bytes=layout().model().serialize();stream.open_chunk(layout_chunk);
    stream.w(bytes.data(),static_cast<u32>(bytes.size()));stream.close_chunk();
} catch (...) {CHECK_OR_EXIT(false,"SQA3 layout save failed");}
void InventoryNextLayoutLoad(IReader& stream) noexcept try {
    layout().reset();auto chunk=stream.open_chunk(layout_chunk);if(!chunk)return;
    CHECK_OR_EXIT(InventoryNextEnabled()&&chunk->length()<=2*1024*1024,"Invalid SQA3 layout mode/size");
    const std::string bytes(reinterpret_cast<const char*>(chunk->pointer()),chunk->length());chunk->close();
    CHECK_OR_EXIT(layout().restore(bytes).ok,"Invalid SQA3 layout data");
} catch (...) {CHECK_OR_EXIT(false,"SQA3 layout load failed");}
void InventoryNextMove(u64 id,int x,int y,bool rotated) noexcept try {
    if(!InventoryNextLayoutSync()||!layout().initialized())return;
    const auto& model=layout().model();
    const auto result=layout().move({id,1,{x,y,rotated},model.revision(),model.session()});
    Msg("* SQA3 layout move %llu: %s",id,result.ok?"accepted":result.error.c_str());
} catch (...) {Msg("! SQA3 layout move exception");}
void InventoryNextProbe(bool export_snapshot,bool show_view) noexcept {
    if(!g_pGameLevel||!Actor()||!InventoryNextLayoutSync())return;
    try {
        const auto& model=layout().model();
        Msg("* SQA3 native layout: items=%u core_grams=%lld",static_cast<unsigned>(model.nodes().size()-2),static_cast<long long>(model.carried_mass()));
        if(show_view) {
            auto ui=CurrentGameUI();
            if(!ui||ui->TopInputReceiver()){Msg("! Close existing game dialogs before opening the SQA3 snapshot");return;}
            ui->ShowActorMenu();
            Msg("* SQA3 normal actor menu opened; shared inventory/loot/equipment interface");
        }
        if(export_snapshot) {
            string_path path; FS.update_path(path,"$app_data_root$","sqa3-native-probe.inv");
            auto saved=inventory_next::save_atomic(model,std::filesystem::path(path));
            if(!saved) Msg("! SQA3 probe export: %s",saved.error.c_str());
            else Msg("* SQA3 probe exported to lab appdata; native item placement exported");
        }
    } catch(const std::exception& e) {
        Msg("! SQA3 probe failed: %s",e.what());
    } catch (...) { Msg("! SQA3 probe failed: unknown core exception"); }
}

bool InventoryNextNativePosition(u16 live,int& x,int& y,bool& rotated) noexcept try {
    if(!InventoryNextEnabled()||!layout().initialized())return false;
    const auto id=InventoryNextIdentity(live);const auto found=layout().model().nodes().find(id);
    if(found==layout().model().nodes().end()||found->second.location!=inventory_next::Location::Grid)return false;
    x=found->second.position.x;y=found->second.position.y;rotated=found->second.position.rotated;return true;
} catch (...) {return false;}
bool InventoryNextNativeDrop(u16 live,int x,int y) noexcept try {
    if(!InventoryNextEnabled()||!InventoryNextLayoutSync())return false;
    const auto id=InventoryNextIdentity(live);const auto& model=layout().model();
    const auto found=model.nodes().find(id);if(found==model.nodes().end()||found->second.location!=inventory_next::Location::Grid)return false;
    const auto result=layout().move({id,1,{x,y,found->second.position.rotated},model.revision(),model.session()});
    Msg("* SQA3 native drag id=%llu destination=%d,%d result=%s",id,x,y,result.ok?"accepted":result.error.c_str());return result.ok;
} catch (...) {return false;}
