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
#include "../inventory_next/storage_catalog.h"
#include "../inventory_next/native_storage_view.h"
#include "ActorBackpack.h"
#include "ui/UIDragDropListEx.h"
#include "ui/UIStatic.h"
#include "string_table.h"
#include "../xrServerEntities/inventory_space.h"
#include "../xrServerEntities/xrMessages.h"
#include "../inventory_next/drag_session.h"
#include "../inventory_next/entity_registry.h"
#include "../inventory_next/persistence.h"
#include <cmath>
#include <exception>
#include <stdexcept>
#include "../inventory_next/ground_pickup.h"
#include "../inventory_next/purchase_session.h"
#include "trade.h"
#include "InventoryOwner.h"
#include "entity_alive.h"
#include "CustomOutfit.h"
#include "script_callback_ex.h"
#include "script_game_object.h"
#include "game_object_space.h"
#include "ai_space.h"
#include "alife_simulator.h"
#include "alife_object_registry.h"

namespace {
inventory_next::ActorLayout& layout(){static inventory_next::ActorLayout value;return value;}
inventory_next::DragSession& native_drag(){static inventory_next::DragSession value;return value;}
constexpr u32 layout_chunk=0x33514c59;
bool layout_current=false;
inventory_next::NativeStorageView storage_view;
std::map<u16,inventory_next::Node> pending_pickups;
std::unique_ptr<inventory_next::EntityRegistry> pickup_registry;
std::unique_ptr<inventory_next::GroundPickup> pickup_transaction;
bool pickup_fault=false;
u16 pickup_source=0xffff;
struct PurchaseRecord { u16 live; u64 identity; u32 price; inventory_next::Node node; };
struct NativePurchase {
    inventory_next::EntityRegistry registry;
    std::unique_ptr<inventory_next::PurchaseSession> session;
    xr_vector<PurchaseRecord> records;
    u16 source=0xffff;u64 source_identity=0,token=0;
    bool committed=false;
};
std::unique_ptr<NativePurchase> purchase;
u64 next_purchase_token=0;


bool read_storage_catalog(inventory_next::StorageCatalog& output) {
    using namespace inventory_next;
    string_path path;FS.update_path(path,"$game_config$","sqa3_capacity.ltx");
    if(!FS.exist(path)){Msg("! SQA3 capacity config missing");return false;}
    CInifile config(path,TRUE,TRUE,FALSE);
    StorageCatalog candidate;
    if(!config.line_exist("sqa3_pockets","size")||!config.line_exist("sqa3_pockets","columns")||
       !config.line_exist("grid_pack_default","size"))return false;
    const int columns=config.r_s32("sqa3_pockets","columns");
    if(!parse_storage_shape(config.r_string("sqa3_pockets","size"),columns,candidate.pockets)||
       !parse_storage_shape(config.r_string("grid_pack_default","size"),7,candidate.default_pack))return false;
    auto read_group=[&](LPCSTR group,std::map<std::string,ActorStorage>& shapes) {
        if(!config.section_exist(group))return true;
        for(u32 i=0;i<config.line_count(group);++i) {
            LPCSTR key=nullptr,value=nullptr;
            if(!config.r_line(group,i,&key,&value)||!key||!value)return false;
            ActorStorage shape;
            if(!parse_storage_shape(value,candidate.default_pack.grid.w,shape))return false;
            shapes.emplace(key,std::move(shape));
        }
        return true;
    };
    if(!read_group("grid_pack_section",candidate.sections)||!read_group("grid_pack_kind",candidate.kinds))return false;
    output=std::move(candidate);return true;
}

}

bool InventoryNextLogicalStorage() noexcept {
    return InventoryNextEnabled()&&strstr(Core.Params,"-sqa3-logical-storage")!=nullptr;
}
bool InventoryNextLayoutSync() noexcept try {
    layout_current=false;
    if(!InventoryNextEnabled()||!g_pGameLevel||!Actor())return true;
    using namespace inventory_next;
    const bool logical=InventoryNextLogicalStorage();
    StorageCatalog catalog;
    if(logical&&!read_storage_catalog(catalog))return false;
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
            if(!n.id) { Msg("! SQA3 awaiting persistent item live=%u section=%s parent=%u",item->object().ID(),item->m_section_id.c_str(),item->parent_id());return false; } InventoryNextIdentityOnline(*item); n.parent=1; n.intrinsic=static_cast<Mass>(std::llround(grams));
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
            if(logical&&smart_cast<CBackpack*>(item)) {
                const std::string section=item->m_section_id.c_str();
                const std::string kind=pSettings->line_exist(section.c_str(),"kind")?pSettings->r_string(section.c_str(),"kind"):"";
                const auto& shape=catalog.resolve(section,kind);
                n.container=true;n.grid=shape.grid;n.cells=shape.cells;
            }
            nodes.push_back(n);
        }
        // Server-acknowledged ownership already reserves storage. Keep the
        // admitted facts until CInventory::Take observes the actual client item.
        for(const auto& pending:pending_pickups) {
            bool present=false;for(const auto& n:nodes)if(n.id==pending.second.id){present=true;break;}
            if(!present){auto n=pending.second;n.parent=1;n.location=Location::Overflow;n.position={};nodes.push_back(std::move(n));}
        }
        auto storage=logical?catalog.pockets:ActorStorage{};storage.logical=logical;
        const auto result=layout().reconcile(std::move(nodes),storage);
        if(!result)Msg("! SQA3 native layout sync: %s",result.error.c_str());
        layout_current=result.ok;
        if(layout_current&&logical)layout_current=storage_view.build(layout().model()).ok;
        return layout_current;
} catch (...) {Msg("! SQA3 native layout sync exception");return false;}
void InventoryNextLayoutReset() noexcept try {layout_current=false;native_drag().cancel();purchase.reset();pickup_transaction.reset();pickup_registry.reset();pending_pickups.clear();pickup_fault=false;layout().reset();} catch (...) {CHECK_OR_EXIT(false,"SQA3 layout reset failed");}
void InventoryNextLayoutSave(IWriter& stream) noexcept try {
    if(!layout().initialized())return;
    const auto bytes=layout().model().serialize();stream.open_chunk(layout_chunk);
    stream.w(bytes.data(),static_cast<u32>(bytes.size()));stream.close_chunk();
} catch (...) {CHECK_OR_EXIT(false,"SQA3 layout save failed");}
void InventoryNextLayoutLoad(IReader& stream) noexcept try {
    layout_current=false;native_drag().cancel();purchase.reset();pickup_transaction.reset();pickup_registry.reset();pending_pickups.clear();pickup_fault=false;layout().reset();auto chunk=stream.open_chunk(layout_chunk);if(!chunk)return;
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
void InventoryNextStore(u64 id,u64 destination,int x,int y,bool rotated) noexcept try {
    if(!InventoryNextLogicalStorage()||!InventoryNextLayoutSync())return;
    const auto& model=layout().model();
    const auto result=layout().move({id,destination,{x,y,rotated},model.revision(),model.session()});
    Msg("* SQA3 logical store id=%llu owner=%llu result=%s",id,destination,result.ok?"accepted":result.error.c_str());
} catch (...) {Msg("! SQA3 logical store exception");}
bool InventoryNextCanRelease(u16 live) noexcept try {
    if(!InventoryNextLogicalStorage())return true;
    if(!pending_pickups.empty()||pickup_transaction||purchase)return false;
    if(!InventoryNextLayoutSync())return false;
    const auto id=InventoryNextIdentity(live);
    if(!id||!layout().model().nodes().count(id))return true;
    for(const auto& pair:layout().model().nodes())if(pair.second.parent==id) {
        Msg("! SQA3 loaded storage release refused live=%u; subtree transfer not ready",live);return false;
    }
    return true;
} catch (...) {return false;}
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
    if(!InventoryNextEnabled()||!layout_current||!layout().initialized())return false;
    const auto id=InventoryNextIdentity(live);const auto found=layout().model().nodes().find(id);
    if(found==layout().model().nodes().end())return false;
    if(InventoryNextLogicalStorage()) {
        inventory_next::Position p;if(!storage_view.position(found->second,p))return false;
        x=p.x;y=p.y;rotated=p.rotated;return true;
    }
    if(found->second.location!=inventory_next::Location::Grid)return false;
    x=found->second.position.x;y=found->second.position.y;rotated=found->second.position.rotated;return true;
} catch (...) {return false;}
int InventoryNextStorageCell(int x,int y) noexcept try {
    if(!layout_current||!InventoryNextLogicalStorage())return 0;
    inventory_next::Id owner=0;inventory_next::Position local;
    if(storage_view.destination(x,y,owner,local))return layout().model().cell_region(owner,local.x,local.y)?1:0;
    return y>=storage_view.recovery_row?2:0;
} catch (...) {return 0;}
void InventoryNextStorageView(CUIDragDropListEx* list) noexcept try {
    if(!InventoryNextLogicalStorage()||!layout_current||!list)return;
    list->SetStartCellsCapacity(Ivector2().set(storage_view.size.w,storage_view.size.h));
    list->SetAutoGrow(false);list->SetGrouping(false);
    for(const auto& band:storage_view.bands) {
        std::string label="Pockets";
        if(band.owner!=1) {
            const auto& n=layout().model().nodes().at(band.owner);
            const auto& section=n.attributes.at("native_section");
            label=pSettings->line_exist(section.c_str(),"inv_name")?
                CStringTable().translate(pSettings->r_string(section.c_str(),"inv_name")).c_str():"Backpack";
            label+=n.location==inventory_next::Location::Equipment?" (equipped)":" (carried)";
        }
        label+="  "+std::to_string(band.grid.w)+" x "+std::to_string(band.grid.h);
        list->AddStorageHeader(band.row-1,label.c_str(),0xffa8d7ef);
    }
    list->AddStorageHeader(storage_view.recovery_row-1,"Recovery - move items into available storage",0xffff9d70);
} catch (...) {Msg("! SQA3 storage view failed");}
bool InventoryNextNativeDrop(u16 live,int x,int y) noexcept try {
    if(!InventoryNextEnabled()||!InventoryNextLayoutSync())return false;
    const auto id=InventoryNextIdentity(live);const auto& model=layout().model();
    const auto found=model.nodes().find(id);if(found==model.nodes().end())return false;
    if(!InventoryNextLogicalStorage()&&found->second.location!=inventory_next::Location::Grid)return false;
    if(native_drag().item()!=id){Msg("! SQA3 native drag missing source token");native_drag().cancel();return false;}
    inventory_next::Id owner=1;inventory_next::Position local{x,y,false};
    if(InventoryNextLogicalStorage()&&!storage_view.destination(x,y,owner,local)) {native_drag().cancel();return false;}
    const auto proposal=native_drag().release(model,owner,local.x,local.y,false,{});
    const auto result=proposal.result.ok ? layout().move(proposal.moves.front()) : proposal.result;
    Msg("* SQA3 native drag id=%llu destination=%d,%d result=%s",id,x,y,result.ok?"accepted":result.error.c_str());return result.ok;
} catch (...) {return false;}
bool InventoryNextNativeDragBegin(u16 live) noexcept try {
    native_drag().cancel();
    if(!InventoryNextEnabled()||!InventoryNextLayoutSync())return false;
    return native_drag().begin(layout().model(),InventoryNextIdentity(live)).ok;
} catch (...) {return false;}
bool InventoryNextNativeDragCurrent(u16 live) noexcept try {
    if(!InventoryNextEnabled()||!InventoryNextLayoutSync())return false;
    const bool valid=native_drag().item()==InventoryNextIdentity(live)&&native_drag().is_current(layout().model());
    if(!valid){Msg("! SQA3 native drag rejected: stale_drag; inventory refreshed");native_drag().cancel();}
    return valid;
} catch (...) {return false;}
bool InventoryNextNativeRotate(u16 live) noexcept try {
    if(!InventoryNextEnabled()||!InventoryNextLayoutSync())return false;
    const auto id=InventoryNextIdentity(live);const auto& model=layout().model();
    const auto found=model.nodes().find(id);
    if(found==model.nodes().end()||found->second.location!=inventory_next::Location::Grid)return false;
    auto position=found->second.position;position.rotated=!position.rotated;
    const auto result=layout().move({id,found->second.parent,position,model.revision(),model.session()});
    Msg("* SQA3 native rotate id=%llu result=%s",id,result.ok?"accepted":result.error.c_str());
    return result.ok;
} catch (...) {return false;}

void InventoryNextCapacityProbe() noexcept try {
    if(!InventoryNextEnabled()||!g_pGameLevel||!Actor()||!InventoryNextLayoutSync())return;
    using namespace inventory_next;
    StorageCatalog catalog;
    if(!read_storage_catalog(catalog)){Msg("! SQA3 capacity catalog rejected");return;}
    auto pack=Actor()->inventory().ItemFromSlot(BACKPACK_SLOT);
    const std::string section=pack?pack->m_section_id.c_str():"";
    const std::string kind=pack&&pSettings->line_exist(section.c_str(),"kind")?pSettings->r_string(section.c_str(),"kind"):"";
    const auto& storage=catalog.resolve(section,kind);
    ActorLayout preview;auto result=preview.restore(layout().model().serialize());
    if(!result){Msg("! SQA3 capacity preview restore: %s",result.error.c_str());return;}
    std::vector<Node> live;
    for(const auto& pair:layout().model().nodes())if(pair.second.kind==Kind::Item)live.push_back(pair.second);
    result=preview.reconcile(std::move(live),storage);
    if(!result){Msg("! SQA3 capacity preview rejected: %s",result.error.c_str());return;}
    unsigned legal=0,recovery=0;
    for(const auto& pair:preview.model().nodes())if(pair.second.kind==Kind::Item) {
        if(pair.second.location==Location::Grid)++legal;
        if(pair.second.location==Location::Overflow)++recovery;
    }
    string_path path;FS.update_path(path,"$app_data_root$","sqa3-capacity-preview.inv");
    result=save_atomic(preview.model(),std::filesystem::path(path));
    Msg("* SQA3 capacity preview pack=%s grid=%dx%d legal=%u recovery=%u grams=%lld export=%s; preflight only",
        section.empty()?"none":section.c_str(),storage.grid.w,storage.grid.h,legal,recovery,
        static_cast<long long>(preview.model().carried_mass()),result.ok?"saved":result.error.c_str());
} catch (...) {Msg("! SQA3 capacity preview exception");}


bool InventoryNextStrictPickup() noexcept {
    return InventoryNextLogicalStorage()&&(strstr(Core.Params,"-sqa3-strict-ground-pickup")||InventoryNextStrictOwnedPickup());
}
bool InventoryNextPickupCanSave() noexcept {
    return !pickup_fault&&!pickup_transaction&&!purchase&&pending_pickups.empty();
}
void InventoryNextPickupForget(u16 live) noexcept try {
    if(purchase&&purchase->session&&purchase->session->state()!=inventory_next::PurchaseSession::State::Prepared)
        for(const auto& record:purchase->records)if(record.live==live)pickup_fault=true;
    pending_pickups.erase(live);
} catch (...) {pickup_fault=true;}
namespace {
bool pickup_slot_compatible(CInventoryItem* item,u16 slot) {
    if(!item||slot==NO_ACTIVE_SLOT)return false;
    const auto base=item->BaseSlot();
    // Match native CanSetItemToList, not just CanPutInSlot (which checks
    // availability but does not constrain actor item types).
    return base==slot||(base==INV_SLOT_2&&(slot==INV_SLOT_3||slot==KNIFE_SLOT))||
        (base==INV_SLOT_3&&slot==INV_SLOT_2);
}
}
bool InventoryNextPickupPrepare(u16 owner,u16 live,u16 source,u16 slot) noexcept try {
    if(!InventoryNextStrictPickup()||!Actor()||owner!=Actor()->ID())return true;
    using namespace inventory_next;
    if(pickup_fault||pickup_transaction||purchase||!InventoryNextLayoutSync())return false;
    auto object=Level().Objects.net_Find(live);
    auto item=smart_cast<CInventoryItem*>(object);
    const auto server=ai().alife().objects().object(live,true);
    if(!item||!server||server->ID_Parent!=source)return false;
    const auto actual_parent=item->object().H_Parent();
    if(source==0xffff?actual_parent!=nullptr:(!actual_parent||actual_parent->ID()!=source))return false;
    if(source!=0xffff&&!InventoryNextStrictOwnedPickup())return false;
    if(!Actor()->inventory().CanTakeItem(item)){InventoryNextPickupStatus("Item cannot be taken. Source unchanged.");return false;}
    if(slot!=0xffff) {
        auto& inventory=Actor()->inventory();
        if(source==0xffff||slot>=48||slot>inventory.LastSlot()||!pickup_slot_compatible(item,slot)||!inventory.CanPutInSlot(item,slot)) {
            Msg("! SQA3 direct equip refused live=%u slot=%u; source retained",live,slot);
            InventoryNextPickupStatus("Equipment slot unavailable. Item stayed at source.");return false;
        }
        // Outfits can displace other equipment. Those compound changes need a
        // separate transaction; never silently push an existing item into recovery.
        auto outfit=smart_cast<CCustomOutfit*>(item);
        if(slot==OUTFIT_SLOT&&outfit&&
           (!inventory.m_belt.empty()||
            (!outfit->bIsHelmetAvaliable&&inventory.ItemFromSlot(HELMET_SLOT))||
            (!outfit->bIsBackpackAvaliable&&inventory.ItemFromSlot(BACKPACK_SLOT)))) {
            InventoryNextPickupStatus("Unequip conflicting gear before taking this outfit.");return false;
        }
    }
    const auto id=InventoryNextIdentity(live);if(!id)return false;
    InventoryNextIdentityOnline(*item);
    Node n;n.id=id;n.parent=2;
    const double grams=double(item->Weight())*1000.0,condition=item->GetCondition();
    const Irect shape=item->GetInvGridRect();
    if(!std::isfinite(grams)||grams<0||grams>1e12||!std::isfinite(condition)||condition<0||condition>1||
       shape.x2<1||shape.y2<1||shape.x2>16||shape.y2>64)return false;
    n.intrinsic=static_cast<Mass>(std::llround(grams));n.condition=static_cast<unsigned>(std::llround(condition*10000));
    n.empty=n.half=n.full={shape.x2,shape.y2};
    n.attributes["native_section"]=item->m_section_id.c_str();n.attributes["native_live_id"]=std::to_string(live);
    n.attributes["native_place"]="3";n.attributes["native_slot"]=std::to_string(item->CurrSlot());
    if(slot!=0xffff) {
        n.wear_slots=std::uint64_t{1}<<slot;
        n.attributes["native_place"]="1";n.attributes["native_slot"]=std::to_string(slot);
    }
    if(smart_cast<CBackpack*>(item)) {
        StorageCatalog catalog;if(!read_storage_catalog(catalog))return false;
        const std::string section=item->m_section_id.c_str();
        const std::string kind=pSettings->line_exist(section.c_str(),"kind")?pSettings->r_string(section.c_str(),"kind"):"";
        const auto& storage=catalog.resolve(section,kind);n.container=true;n.grid=storage.grid;n.cells=storage.cells;
    }
    auto registry=std::make_unique<EntityRegistry>();
    if(!registry->add(1)||!registry->bind(1,owner))return false;
    std::vector<Id> destinations{1};
    for(const auto& pair:layout().model().nodes())if(pair.second.kind==Kind::Item) {
        if(!registry->add(pair.first)||!registry->bind(pair.first,static_cast<u16>(std::stoul(pair.second.attributes.at("native_live_id")))))return false;
        if(pair.second.container&&pair.second.location!=Location::Overflow&&layout().model().storage_active(pair.first))destinations.push_back(pair.first);
    }
    if(source!=0xffff&&(!registry->add(2)||!registry->bind(2,source)))return false;
    if(!registry->add(id)||!registry->bind(id,live))return false;
    auto transaction=std::make_unique<GroundPickup>(*registry);
    const auto result=transaction->prepare(layout().model(),n,destinations,source==0xffff,slot==0xffff?0:n.wear_slots);
    if(!result){Msg("! SQA3 %s pickup refused live=%u reason=%s; source retained",source==0xffff?"ground":"owned",live,result.error.c_str());InventoryNextPickupStatus("No space. Item stayed at source.");return false;}
    pickup_registry=std::move(registry);pickup_transaction=std::move(transaction);pickup_source=source;
    if(slot!=0xffff)Msg("* SQA3 direct equip reserved live=%u slot=%u",live,slot);
    Msg("* SQA3 %s pickup reserved live=%u; save blocked=%d",source==0xffff?"ground":"owned",live,!InventoryNextPickupCanSave());
    return true;
} catch (...) {Msg("! SQA3 ground pickup prepare exception; source retained");return false;}
void InventoryNextPickupFinish(u16 owner,u16 live,bool accepted) noexcept try {
    if(!InventoryNextStrictPickup()||!Actor()||owner!=Actor()->ID()||!pickup_transaction)return;
    const auto server=ai().alife().objects().object(live,true);
    const auto result=pickup_transaction->acknowledge(server?server->ID_Parent:0xffff,accepted);
    if(accepted&&result) {
        const auto id=InventoryNextIdentity(live);
        const auto admitted=pickup_transaction->model().nodes().at(id);
        // Restore only the acknowledged actor forest, never a rejected source.
        const auto restored=layout().restore(pickup_transaction->model().serialize());
        if(!restored){pickup_fault=true;Msg("! SQA3 pickup publish failed: %s",restored.error.c_str());return;}
        pending_pickups.emplace(live,admitted);
        InventoryNextPickupStatus("");
        if(InventoryNextIdentityCanSave()){pickup_fault=true;Msg("! SQA3 pending pickup incorrectly allowed save");return;}
        Msg("* SQA3 %s pickup server accepted live=%u owner=%llu cell=%d,%d",pickup_source==0xffff?"ground":"owned",live,admitted.parent,admitted.position.x,admitted.position.y);
    } else if(accepted||result.error!="native_rejected") {
        pickup_fault=true;Msg("! SQA3 pickup acknowledgement failed: %s",result.error.c_str());return;
    }
    pickup_transaction.reset();pickup_registry.reset();
} catch (...) {pickup_fault=true;Msg("! SQA3 pickup acknowledgement exception; saves blocked");}
void InventoryNextPickupTaken(u16 live) noexcept try {
    if(!pending_pickups.count(live))return;
    const auto expected=pending_pickups.at(live);
    pending_pickups.erase(live);
    if(!InventoryNextLayoutSync()){pickup_fault=true;return;}
    const auto id=InventoryNextIdentity(live);const auto found=layout().model().nodes().find(id);
    if(found==layout().model().nodes().end()||found->second.location==inventory_next::Location::Overflow||
       (expected.location==inventory_next::Location::Equipment&&
        (found->second.location!=expected.location||found->second.equipped_slots!=expected.equipped_slots))) {
        pickup_fault=true;Msg("! SQA3 ordinary pickup lost reserved placement; saves blocked");return;
    }
    InventoryNextPickupStatus(nullptr);
    Msg("* SQA3 pickup client acknowledged live=%u; pending=%u save_ready=%d",live,static_cast<unsigned>(pending_pickups.size()),InventoryNextPickupCanSave());
} catch (...) {pickup_fault=true;}

u16 InventoryNextPickupSlot(u16 owner,u16 live) noexcept try {
    if(!Actor()||owner!=Actor()->ID())return 0xffff;
    const auto pending=pending_pickups.find(live);
    if(pending==pending_pickups.end()||pending->second.location!=inventory_next::Location::Equipment)return 0xffff;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
    for(u16 slot=1;slot<48;++slot)if(pending->second.equipped_slots==(std::uint64_t{1}<<slot)) {
        if(item&&slot<=Actor()->inventory().LastSlot()&&pickup_slot_compatible(item,slot)&&Actor()->inventory().CanPutInSlot(item,slot))return slot;
        break;
    }
    pickup_fault=true;InventoryNextPickupStatus("Equipment changed during transfer. Saving blocked.");return 0xffff;
} catch (...) {pickup_fault=true;return 0xffff;}


bool InventoryNextStrictOwnedPickup() noexcept {
    return InventoryNextLogicalStorage()&&(strstr(Core.Params,"-sqa3-strict-owned-pickup")!=nullptr||InventoryNextStrictTradePickup());
}
void InventoryNextPickupStatus(const char* text) noexcept try {
    if(!CurrentGameUI())return;
    auto& menu=CurrentGameUI()->GetActorMenu();menu.m_sqaRefresh=true;menu.m_sqaRetryAfter=Device.dwTimeGlobal+250;
    if(text&&menu.m_sqaStatus)menu.m_sqaStatus->TextItemControl()->SetText(text);
} catch (...) {Msg("! SQA3 acquisition status refresh failed");}
bool InventoryNextRequestOwnedPickup(u16 source,u16 owner,u16 live,u16 slot) noexcept try {
    if(!InventoryNextStrictOwnedPickup()||!Actor()||owner!=Actor()->ID()||source==owner||source==0xffff)return false;
    const auto source_identity=InventoryNextIdentity(source),item_identity=InventoryNextIdentity(live);
    if(!source_identity||!item_identity){InventoryNextPickupStatus("Item unavailable. Transfer cancelled.");return true;}
    NET_Packet packet;CGameObject::u_EventGen(packet,GE_SQA_ACQUIRE,owner);
    packet.w_u16(source);packet.w_u16(live);packet.w_u64(source_identity);packet.w_u64(item_identity);
    packet.w_u16(slot);
    // Local single-player dispatch may finish synchronously. Set the pending
    // label first so the resulting refusal/acceptance remains visible.
    InventoryNextPickupStatus("Checking storage...");CGameObject::u_EventSend(packet);return true;
} catch (...) {InventoryNextPickupStatus("Transfer unavailable. Item stayed at source.");return true;}

bool InventoryNextStrictTradePickup() noexcept {
    return InventoryNextLogicalStorage()&&strstr(Core.Params,"-sqa3-strict-trade-pickup")!=nullptr;
}
void InventoryNextPurchaseProbe(u16 source,const xr_vector<u16>& items) noexcept try {
    if(!InventoryNextStrictTradePickup()||!strstr(Core.Params,"-sqa3-fixture"))return;
    auto seller=smart_cast<CInventoryOwner*>(Level().Objects.net_Find(source));
    if(!seller||!seller->GetTrade()){Msg("! SQA3 purchase fixture source missing");return;}
    InventoryNextPurchaseRequest(seller->GetTrade(),items);
} catch (...) {Msg("! SQA3 purchase fixture failed");}
namespace {
CInventoryOwner* purchase_seller(u16 source) {
    return smart_cast<CInventoryOwner*>(Level().Objects.net_Find(source));
}
bool purchase_node(u16 live,u16 source,inventory_next::Node& n) {
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
    auto server=ai().alife().objects().object(live,true);
    if(!item||!server||server->ID_Parent!=source||!item->object().H_Parent()||item->parent_id()!=source||
       !item->CanTrade()||!Actor()->inventory().CanTakeItem(item))return false;
    const auto id=InventoryNextIdentity(live);if(!id)return false;
    InventoryNextIdentityOnline(*item);
    const double grams=double(item->Weight())*1000.0,condition=item->GetCondition();const auto shape=item->GetInvGridRect();
    if(!std::isfinite(grams)||grams<0||grams>1e12||!std::isfinite(condition)||condition<0||condition>1||
       shape.x2<1||shape.y2<1||shape.x2>16||shape.y2>64)return false;
    n.id=id;n.parent=2;n.intrinsic=static_cast<inventory_next::Mass>(std::llround(grams));
    n.condition=static_cast<unsigned>(std::llround(condition*10000));n.empty=n.half=n.full={shape.x2,shape.y2};
    n.attributes["native_section"]=item->m_section_id.c_str();n.attributes["native_live_id"]=std::to_string(live);
    n.attributes["native_place"]="3";n.attributes["native_slot"]=std::to_string(item->CurrSlot());
    if(smart_cast<CBackpack*>(item)) {
        inventory_next::StorageCatalog catalog;if(!read_storage_catalog(catalog))return false;
        const std::string section=item->m_section_id.c_str();
        const std::string kind=pSettings->line_exist(section.c_str(),"kind")?pSettings->r_string(section.c_str(),"kind"):"";
        const auto& storage=catalog.resolve(section,kind);n.container=true;n.grid=storage.grid;n.cells=storage.cells;
    }
    return true;
}
bool same_purchase_node(const inventory_next::Node& a,const inventory_next::Node& b) {
    return a.id==b.id&&a.intrinsic==b.intrinsic&&a.condition==b.condition&&a.empty.w==b.empty.w&&a.empty.h==b.empty.h&&
        a.container==b.container&&a.grid.w==b.grid.w&&a.grid.h==b.grid.h&&a.cells==b.cells&&a.attributes==b.attributes;
}
void purchase_refused(const char* text) {
    purchase.reset();InventoryNextPickupStatus(text);
}
}
bool InventoryNextPurchaseRequest(CTrade* trade,const xr_vector<u16>& items) noexcept try {
    if(!InventoryNextStrictTradePickup())return false;
    if(!Actor()||!trade||trade->GetPartner()!=Actor()||purchase||!InventoryNextPickupCanSave()||!InventoryNextLayoutSync()) {
        InventoryNextPickupStatus("Purchase unavailable. Finish the pending transfer first.");return true;
    }
    auto seller=trade->pThis.inv_owner;auto source_object=smart_cast<CGameObject*>(seller);
    if(!seller||!source_object||items.empty()||items.size()>512){InventoryNextPickupStatus("Purchase source or selection unavailable.");return true;}
    auto creature=smart_cast<CEntityAlive*>(source_object);
    if((creature&&!creature->g_Alive())||
       (source_object->Position().distance_to(Actor()->Position())>3.0f&&!seller->NeedOsoznanieMode())) {
        InventoryNextPickupStatus("Seller out of reach. Purchase cancelled.");return true;
    }
    auto prepared=std::make_unique<NativePurchase>();prepared->source=source_object->ID();
    prepared->source_identity=InventoryNextIdentity(prepared->source);if(!prepared->source_identity)return true;
    auto& registry=prepared->registry;
    if(!registry.add(1)||!registry.bind(1,Actor()->ID())||!registry.add(2)||!registry.bind(2,prepared->source))return true;
    std::vector<inventory_next::Id> destinations{1};
    for(const auto& pair:layout().model().nodes())if(pair.second.kind==inventory_next::Kind::Item) {
        if(!registry.add(pair.first)||!registry.bind(pair.first,static_cast<u16>(std::stoul(pair.second.attributes.at("native_live_id")))))return true;
        if(pair.second.container&&pair.second.location!=inventory_next::Location::Overflow&&layout().model().storage_active(pair.first))destinations.push_back(pair.first);
    }
    std::vector<inventory_next::PurchaseSession::Line> lines;
    for(u16 live:items) {
        inventory_next::Node node;if(!purchase_node(live,prepared->source,node)||!registry.add(node.id)||!registry.bind(node.id,live)) {
            InventoryNextPickupStatus("Purchase changed. Items stayed with seller.");return true;
        }
        auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));const u32 price=trade->GetItemPrice(item,false);
        lines.push_back({node,price});prepared->records.push_back({live,node.id,price,node});
    }
    prepared->session=std::make_unique<inventory_next::PurchaseSession>(layout().model(),registry);
    auto result=prepared->session->prepare(lines,destinations,{Actor()->get_money(),seller->get_money()});
    if(!result){Msg("! SQA3 purchase refused before callbacks reason=%s",result.error.c_str());InventoryNextPickupStatus(result.error=="no_fit"?"Purchase does not fit. Items and money unchanged.":"Purchase unavailable. Items and money unchanged.");return true;}
    prepared->token=++next_purchase_token;purchase=std::move(prepared);
    u32 total=0;for(const auto& record:purchase->records)total+=record.price;
    // Preserve native before-trade callbacks. The server revalidates everything
    // afterward; callbacks cannot authorize stale prices, owners or placements.
    const auto records=purchase->records;
    trade->OnPerformTrade(total,0);
    if(!purchase)return true;
    for(const auto& record:records) {
        seller=purchase_seller(purchase->source);
        auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(record.live));
        if(!seller||InventoryNextIdentity(purchase->source)!=purchase->source_identity||!item||item->object().getDestroy()||
           InventoryNextIdentity(record.live)!=record.identity){purchase_refused("Purchase changed. Transfer cancelled.");return true;}
        seller->on_before_sell(item);
        if(!purchase||!Actor())return true;
        item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(record.live));
        if(!item||item->object().getDestroy()||InventoryNextIdentity(record.live)!=record.identity){purchase_refused("Purchase changed. Transfer cancelled.");return true;}
        Actor()->on_before_buy(item);
        if(!purchase||!Actor())return true;
    }
    NET_Packet packet;CGameObject::u_EventGen(packet,GE_SQA_PURCHASE,Actor()->ID());
    packet.w_u16(purchase->source);packet.w_u64(purchase->token);
    InventoryNextPickupStatus("Checking purchase...");CGameObject::u_EventSend(packet);
    return true;
} catch (...) {
    if(purchase&&purchase->session&&purchase->session->state()!=inventory_next::PurchaseSession::State::Prepared)pickup_fault=true;
    else purchase_refused("Purchase unavailable. Transfer cancelled.");
    return true;
}
bool InventoryNextPurchaseValidate(u16 source,u16 owner,u64 token,xr_vector<u16>& items) noexcept try {
    if(!InventoryNextStrictTradePickup()||!purchase||purchase->token!=token||purchase->source!=source||!Actor()||Actor()->ID()!=owner)return false;
    auto seller=purchase_seller(source);
    if(!seller||!ai().alife().objects().object(source,true)||InventoryNextIdentity(source)!=purchase->source_identity||!seller->GetTrade()||seller->GetTrade()->GetPartner()!=Actor()) {
        purchase_refused("Seller unavailable. Purchase cancelled.");return false;
    }
    auto source_object=smart_cast<CGameObject*>(seller);
    auto creature=smart_cast<CEntityAlive*>(source_object);
    if((creature&&!creature->g_Alive())||!source_object||
       (source_object->Position().distance_to(Actor()->Position())>3.0f&&!seller->NeedOsoznanieMode())) {
        purchase_refused("Seller out of reach. Purchase cancelled.");return false;
    }
    for(const auto& record:purchase->records) {
        inventory_next::Node observed;
        auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(record.live));
        if(!purchase_node(record.live,source,observed)||!same_purchase_node(observed,record.node)||
           seller->GetTrade()->GetItemPrice(item,false)!=record.price) {
            purchase_refused("Purchase changed. Items stayed with seller.");return false;
        }
        items.push_back(record.live);
    }
    const auto result=purchase->session->dispatch(true,{Actor()->get_money(),seller->get_money()});
    if(!result){purchase_refused("Purchase changed. Items stayed with seller.");return false;}
    return true;
} catch (...) {purchase_refused("Purchase validation failed. Transfer cancelled.");return false;}
bool InventoryNextPurchaseFinish(bool accepted) noexcept try {
    if(!purchase||!Actor())return false;
    auto seller=purchase_seller(purchase->source);if(!seller){pickup_fault=true;return false;}
    std::map<inventory_next::Id,u16> parents;
    for(const auto& record:purchase->records) {
        auto server=ai().alife().objects().object(record.live,true);
        parents.emplace(record.identity,server?server->ID_Parent:0xffff);
    }
    auto result=purchase->session->acknowledge(parents,accepted,{Actor()->get_money(),seller->get_money()});
    if(!result) {
        if(purchase->session->can_save())purchase_refused("Purchase refused. Items and money unchanged.");
        else {pickup_fault=true;InventoryNextPickupStatus("Purchase needs reconciliation. Saving blocked.");}
        return false;
    }
    const auto money=purchase->session->settlement();
    Actor()->set_money(money.actor,false);seller->set_money(money.seller,false);
    result=purchase->session->settled({Actor()->get_money(),seller->get_money()});
    if(!result){pickup_fault=true;return false;}
    const auto committed=purchase->session->committed_model();
    if(!committed||!layout().restore(committed->serialize())){pickup_fault=true;return false;}
    for(const auto& record:purchase->records)pending_pickups.emplace(record.live,committed->nodes().at(record.identity));
    purchase->committed=true;
    if(InventoryNextIdentityCanSave()){pickup_fault=true;return false;}
    Actor()->set_money(Actor()->get_money(),true);seller->set_money(seller->get_money(),true);
    Msg("* SQA3 purchase committed count=%u actor_money=%u seller_money=%u; client acknowledgement pending",static_cast<unsigned>(purchase->records.size()),money.actor,money.seller);
    InventoryNextPickupStatus("");return true;
} catch (...) {pickup_fault=true;return false;}
void InventoryNextPurchaseClientReady() noexcept try {
    if(!purchase||!purchase->committed||!pending_pickups.empty()||pickup_fault)return;
    const auto records=purchase->records;
    for(const auto& record:records) {
        auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(record.live));
        if(!item||InventoryNextIdentity(record.live)!=record.identity||item->parent_id()!=Actor()->ID()){pickup_fault=true;return;}
    }
    purchase.reset();
    if(CurrentGameUI())CurrentGameUI()->GetActorMenu().SqaPurchaseRefresh();
    for(const auto& record:records) {
        auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(record.live));
        if(item&&InventoryNextIdentity(record.live)==record.identity)
            Actor()->callback(GameObject::eTradeSellBuyItem)(item->object().lua_game_object(),false,record.price);
    }
    Msg("* SQA3 purchase client completed count=%u save_ready=%d",static_cast<unsigned>(records.size()),InventoryNextPickupCanSave());
} catch (...) {pickup_fault=true;}
