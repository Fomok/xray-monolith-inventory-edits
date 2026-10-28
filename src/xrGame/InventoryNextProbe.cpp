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
#include "../inventory_next/view_metrics.h"
#include "ActorBackpack.h"
#include "ui/UIDragDropListEx.h"
#include "ui/UIStatic.h"
#include "string_table.h"
#include "../xrServerEntities/inventory_space.h"
#include "../xrServerEntities/xrMessages.h"
#include "../xrServerEntities/xrServer_Objects_ALife_Items.h"
#include "../inventory_next/drag_session.h"
#include "../inventory_next/entity_registry.h"
#include "../inventory_next/persistence.h"
#include <cmath>
#include <exception>
#include <stdexcept>
#include "../inventory_next/ground_pickup.h"
#include "../inventory_next/purchase_session.h"
#include "../inventory_next/world_subtrees.h"
#include "../inventory_next/loaded_storage_session.h"
#include "InventoryBox.h"
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
#include "ai_object_location.h"
#include "map_manager.h"
#include "map_location.h"
#include "script_engine.h"
#include "../inventory_next/equipment_registry.h"
#include "../inventory_next/quick_access.h"
#include "Weapon.h"
#include "WeaponAmmo.h"
#include "eatable_item.h"
#include "UsableScriptObject.h"

namespace {
inventory_next::ActorLayout& layout(){static inventory_next::ActorLayout value;return value;}
inventory_next::WorldSubtrees& world_storage(){static inventory_next::WorldSubtrees value;return value;}
inventory_next::DragSession& native_drag(){static inventory_next::DragSession value;return value;}
constexpr u32 layout_chunk=0x33514c59;
constexpr u32 world_storage_chunk=0x33515753;
bool layout_current=false;
bool equipment_dirty=false;
std::set<u64> dismantling_items;
// Filled during native source enumeration, never during rendering. HD icon
// dimensions remain UV coordinates; they are not storage footprints.
std::map<u64,inventory_next::Size> source_footprints;
inventory_next::EquipmentRegistry equipment_registry;
bool equipment_registry_ready=false;
bool rig_wear_enabled=false;
float rig_wear_per_hit=0.0f,rig_wear_per_second=0.0f,rig_wear_spent=0.0f;
u32 rig_wear_started=0;
bool medical_rig_only=true,medical_menu_exception=true,ammo_rig_only=true;
std::string medical_kinds="i_medical";
u32 access_updated_at=0,access_inventory_frame=0;
std::shared_ptr<const inventory_next::StorageCatalog> storage_catalog_cache;
struct PendingCustomBinding {u64 item=0,host=0;u32 slot=0;};
std::vector<PendingCustomBinding> pending_custom_bindings;
bool restoring_custom_bindings=false;
bool fieldkit_equipped_view=true;
inventory_next::NativeStorageView storage_view;
std::set<u64> collapsed_storage;
std::map<u64,inventory_next::NativeStorageView> storage_views;
inventory_next::StorageViewOptions view_options(u64 owner=0) {
    inventory_next::StorageViewOptions options;
    options.center=InventoryNextFieldKitEnabled();
    options.equipped_only=options.center&&fieldkit_equipped_view;
    options.owner=owner;if(!owner)options.collapsed=collapsed_storage;return options;
}
const inventory_next::NativeStorageView& make_storage_view(u64 owner) {
    const auto cached=storage_views.find(owner);if(cached!=storage_views.end())return cached->second;
    inventory_next::NativeStorageView view;
    const auto world=world_storage().records().find(owner);
    const auto& model=world==world_storage().records().end()?layout().model():world->second.tree;
    if(!view.build(model,view_options(owner)))throw std::runtime_error("storage view owner unavailable");
    return storage_views.emplace(owner,std::move(view)).first->second;
}

std::map<u16,inventory_next::Node> pending_pickups;
std::unique_ptr<inventory_next::EntityRegistry> pickup_registry;
std::unique_ptr<inventory_next::GroundPickup> pickup_transaction;
std::unique_ptr<inventory_next::WorldSubtrees> pickup_world_candidate;
std::string pickup_world_before;
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
struct NativeWorldTransfer {
    inventory_next::EntityRegistry registry;
    std::unique_ptr<inventory_next::LoadedStorageSession> session;
    inventory_next::TransferSession::Dispatch dispatch;
    xr_vector<PurchaseRecord> records;
    u16 source=0xffff,destination=0xffff;
    u64 custodian=0,token=0;
    bool acquire=false,committed=false;
    bool created_ground_proxy=false,retire_ground_proxy=false;
    std::string stash_name;u16 stash_shell=0xffff;
};
std::unique_ptr<NativeWorldTransfer> world_transfer;
u64 next_world_token=0;
struct NativeWorldMember {
    inventory_next::Model actor_candidate;
    inventory_next::WorldSubtrees world_candidate;
    inventory_next::Node observed;
    std::string actor_before,world_before;
    u16 source=0xffff,destination=0xffff,live=0xffff;
    u64 source_identity=0,destination_identity=0,item_identity=0,source_shell=0,target_shell=0,token=0;
    bool actor_source=false,committed=false;
};
std::unique_ptr<NativeWorldMember> world_member;
u64 next_world_member_token=0;
struct NativeGroundProvision {
    u16 shell=0xffff,proxy=0xffff;
    u64 shell_identity=0;
    u32 started=0;
    std::string actor_before,stash_name;
};
std::unique_ptr<NativeGroundProvision> ground_provision;
constexpr LPCSTR ground_proxy_section="sqa3_ground_backpack";
std::map<u64,u16> stash_markers;


bool read_storage_catalog(inventory_next::StorageCatalog& output) {
    using namespace inventory_next;
    if(storage_catalog_cache){output=*storage_catalog_cache;return true;}
    string_path path;FS.update_path(path,"$game_config$","sqa3_capacity.ltx");
    if(!FS.exist(path)){Msg("! SQA3 capacity config missing");return false;}
    CInifile config(path,TRUE,TRUE,FALSE);
    if(config.section_exist("sqa3_catalog_metadata")&&
       (!config.line_exist("sqa3_catalog_metadata","schema_version")||config.r_u32("sqa3_catalog_metadata","schema_version")!=1)){
        Msg("! SQA3 unsupported storage definition schema; use the local definition validator");return false;
    }
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
            if(value[0]=='@') {
                const LPCSTR section=value+1;
                if(!config.section_exist(section)||!config.line_exist(section,"size")||
                    !parse_storage_shape(config.r_string(section,"size"),candidate.default_pack.grid.w,shape))return false;
                if(config.line_exist(section,"mask")&&!parse_storage_mask(config.r_string(section,"mask"),shape))return false;
            } else if(!parse_storage_shape(value,candidate.default_pack.grid.w,shape))return false;
            shapes.emplace(key,std::move(shape));
        }
        return true;
    };
    if(!read_group("grid_pack_section",candidate.sections)||!read_group("grid_pack_kind",candidate.kinds))return false;
    if(config.section_exist("sqa3_storage_definitions"))for(u32 i=0;i<config.line_count("sqa3_storage_definitions");++i) {
        LPCSTR item=nullptr,section=nullptr;
        if(!config.r_line("sqa3_storage_definitions",i,&item,&section)||!item||!section||!config.section_exist(section))return false;
        StorageDefinition definition;
        for(const auto key:{"size","mask","family","empty","half","full","growing","allow_container_items"})
            if(!config.line_exist(section,key))return false;
        if(!parse_storage_shape(config.r_string(section,"size"),7,definition.interior)||
            !parse_storage_mask(config.r_string(section,"mask"),definition.interior))return false;
        auto exterior=[&](LPCSTR key,Size& size){ActorStorage shape;if(!parse_storage_shape(config.r_string(section,key),7,shape))return false;size=shape.grid;return true;};
        if(!exterior("empty",definition.empty)||!exterior("half",definition.half)||!exterior("full",definition.full))return false;
        definition.family=config.r_string(section,"family");
        definition.growing=config.r_bool(section,"growing");definition.allow_container_items=config.r_bool(section,"allow_container_items");
        if(config.line_exist(section,"allow_rig_items"))definition.allow_rig_items=config.r_bool(section,"allow_rig_items");
        if(config.line_exist(section,"exterior_independent"))definition.independent_exterior=config.r_bool(section,"exterior_independent");
        if(config.line_exist(section,"max_contents_grams")){
            const auto raw=config.r_string(section,"max_contents_grams");if(!raw)return false;
            std::size_t consumed=0;definition.max_contents=std::stoll(raw,&consumed);
            if(raw[consumed]||definition.max_contents>1000000000000LL)return false;
        }
        if(config.line_exist(section,"stack_limit"))definition.stack_limit=config.r_u32(section,"stack_limit");
        if(config.line_exist(section,"accept_kinds")) {
            auto text=[&](LPCSTR key){const auto value=config.r_string(section,key);return std::string(value?value:"");};
            definition.restricted=true;definition.accept_kinds=text("accept_kinds");
            for(const auto key:{"accept_sections","reject_sections","accept_flags"})if(!config.line_exist(section,key))return false;
            definition.accept_sections=text("accept_sections");
            definition.reject_sections=text("reject_sections");
            definition.accept_flags=text("accept_flags");
        }
        if(config.line_exist(section,"tear_thresholds")) {
            std::istringstream thresholds(config.r_string(section,"tear_thresholds"));std::string value;unsigned previous=10001;
            while(std::getline(thresholds,value,',')) {
                const auto last=value.find_last_not_of(" \t");if(last==std::string::npos)return false;
                value.resize(last+1);std::size_t consumed=0;const auto threshold=std::stoul(value,&consumed);
                if(consumed!=value.size())return false;
                if(threshold>=previous||threshold>10000||definition.tear_thresholds.size()>=64)return false;
                definition.tear_thresholds.push_back(static_cast<unsigned>(threshold));previous=static_cast<unsigned>(threshold);
            }
            if(definition.tear_thresholds.empty()||definition.family!="rig")return false;
        }
        if(definition.max_contents<0||definition.stack_limit>1000000||definition.family.empty())return false;
        if(!candidate.definitions.emplace(item,std::move(definition)).second)return false;
    }
    storage_catalog_cache=std::make_shared<const StorageCatalog>(std::move(candidate));output=*storage_catalog_cache;return true;
}
bool read_equipment_registry() {
    using namespace inventory_next;
    if(equipment_registry_ready)return true;
    string_path path;FS.update_path(path,"$game_config$","sqa3_equipment.ltx");
    EquipmentRegistry candidate;
    bool wear_enabled=false;float per_hit=0.0f,per_second=0.0f;
    if(FS.exist(path)) {
        CInifile config(path,TRUE,TRUE,FALSE);
        if(!config.section_exist("slots"))return false;
        for(u32 i=0;i<config.line_count("slots");++i) {
            LPCSTR key=nullptr,section=nullptr;if(!config.r_line("slots",i,&key,&section)||!key||!section||!config.section_exist(section))return false;
            for(const auto field:{"id","families","mounted","enabled","model_bit","minimum_condition"})if(!config.line_exist(section,field))return false;
            EquipmentSlotDefinition slot;slot.key=key;slot.id=config.r_u32(section,"id");slot.families=config.r_string(section,"families");
            slot.label=config.line_exist(section,"label")?config.r_string(section,"label"):"Equipment";
            slot.mounted=config.r_bool(section,"mounted");slot.enabled=config.r_bool(section,"enabled");
            slot.model_bit=config.r_u32(section,"model_bit");slot.minimum_condition=config.r_u32(section,"minimum_condition");
            if(!candidate.add(std::move(slot)))return false;
        }
        if(config.section_exist("rig_wear")) {
            wear_enabled=config.r_bool("rig_wear","enabled");
            per_hit=config.r_float("rig_wear","per_hit");per_second=config.r_float("rig_wear","per_second");
            if(!std::isfinite(per_hit)||!std::isfinite(per_second)||per_hit<0||per_hit>1||per_second<0||per_second>1)return false;
        }
        if(config.section_exist("quick_access")) {
            medical_rig_only=config.r_bool("quick_access","medical_rig_only");
            medical_menu_exception=config.r_bool("quick_access","medical_menu_exception");
            ammo_rig_only=config.r_bool("quick_access","ammo_rig_only");
            medical_kinds=config.r_string("quick_access","medical_kinds");
        }
    }
    rig_wear_enabled=wear_enabled;rig_wear_per_hit=per_hit;rig_wear_per_second=per_second;rig_wear_spent=0;rig_wear_started=Device.dwTimeGlobal;
    equipment_registry=std::move(candidate);equipment_registry_ready=true;return true;
}
void native_instance_state(inventory_next::Node& node,CInventoryItem* item) {
    // One native entity is one independently stateful member. Rounds/charges
    // inside it are quantities, never extra inventory identities or fake cells.
    node.quantity=node.stack_limit=1;node.attributes["native_quantity_domain"]="instance";
    if(item->m_kind.size())node.attributes["native_kind"]=item->m_kind.c_str();
    if(auto ammo=smart_cast<CWeaponAmmo*>(item)){
        node.attributes["native_is_ammo"]="1";
        node.attributes["native_rounds"]=std::to_string(ammo->m_boxCurr);
        node.attributes["native_round_capacity"]=std::to_string(ammo->m_boxSize);
    }
    if(auto weapon=smart_cast<CWeapon*>(item)){
        node.attributes["native_is_weapon"]="1";
        node.attributes["native_loaded_rounds"]=std::to_string(weapon->GetAmmoElapsed());
        node.attributes["native_addons"]=std::to_string(weapon->GetAddonsState());
    }
    if(auto eatable=smart_cast<CEatableItem*>(item)){
        node.attributes["native_uses"]=std::to_string(eatable->GetRemainingUses());
        node.attributes["native_max_uses"]=std::to_string(eatable->GetMaxUses());
    }
    std::string upgrades;
    for(const auto& upgrade:item->upgardes()){if(!upgrades.empty())upgrades+='|';upgrades+=upgrade.c_str();}
    if(!upgrades.empty())node.attributes["native_upgrades"]=upgrades;
    if(pSettings->line_exist(item->m_section_id,"is_mag")&&pSettings->r_bool(item->m_section_id,"is_mag"))node.attributes["native_is_magazine"]="1";
}
void apply_custom_binding(inventory_next::Node& node,CInventoryItem* item) {
    using namespace inventory_next;
    const auto family=node.attributes.find("storage_family");
    if(family!=node.attributes.end()) {
        node.pouch=family->second=="pouch";
        if(family->second=="rig")node.attributes["native_is_rig"]="1";
        for(const auto& entry:equipment_registry.slots()) {
            const auto& slot=entry.second;
            if(!slot.mounted&&slot.enabled&&slot.accepts(family->second))node.wear_slots|=std::uint64_t{1}<<slot.model_bit;
            if(slot.mounted&&family->second=="rig")node.mounts.push_back({slot.id,"custom_mount_"+std::to_string(slot.id),{1,1},~std::uint64_t{0},slot.minimum_condition,slot.enabled});
        }
    }
    u64 host=0;u32 id=0;
    if(!Actor()->inventory().CustomSlotFor(item,host,id))return;
    const auto slot=equipment_registry.find(id);if(!slot)return;
    node.attributes["native_custom_slot"]=std::to_string(id);node.attributes["native_custom_host"]=std::to_string(host);node.attributes["native_custom_key"]=slot->key;
    node.parent=host;node.position={};
    if(slot->mounted){node.location=Location::Mounted;node.mounted_slot=id;node.equipped_slots=0;}
    else {node.location=Location::Equipment;node.wear_slots|=std::uint64_t{1}<<slot->model_bit;node.equipped_slots=std::uint64_t{1}<<slot->model_bit;}
}
void queue_custom_bindings() {
    using namespace inventory_next;
    pending_custom_bindings.clear();
    for(const auto& entry:layout().model().nodes()) {
        const auto& n=entry.second;
        if(n.location==Location::Mounted&&n.mounted_slot>65535&&n.mounted_slot<0xffffffffULL)
            pending_custom_bindings.push_back({n.id,n.parent,static_cast<u32>(n.mounted_slot)});
        else if(n.location==Location::Equipment&&n.attributes.count("native_custom_slot")) {
            const auto slot=std::stoull(n.attributes.at("native_custom_slot"));
            if(slot<=65535||slot>=0xffffffffULL)throw std::runtime_error("invalid custom slot domain");
            pending_custom_bindings.push_back({n.id,std::stoull(n.attributes.at("native_custom_host")),static_cast<u32>(slot)});
        }
    }
}

}

bool InventoryNextLogicalStorage() noexcept {
    return InventoryNextEnabled()&&strstr(Core.Params,"-sqa3-logical-storage")!=nullptr;
}
void InventoryNextCustomChanged() noexcept {layout_current=false;storage_views.clear();InventoryNextPickupStatus(nullptr);}
void InventoryNextConditionChanged(u16 live) noexcept try {
    if(!InventoryNextLogicalStorage())return;
    // ALife persists the server item condition and restores it on the next
    // online spawn. Keep the actor-owned server fact in step with native wear,
    // repair and script condition changes while this isolated mode is active.
    if(Actor()&&g_pGameLevel){
        auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
        auto server=ai().alife().objects().object(live,true);
        auto server_item=smart_cast<CSE_ALifeInventoryItem*>(server);
        if(item&&server_item&&item->parent_id()==Actor()->ID()&&server->ID_Parent==Actor()->ID()){
            const float condition=item->GetCondition();
            if(std::isfinite(condition)&&condition>=0.0f&&condition<=1.0f)server_item->m_fCondition=condition;
        }
    }
    if(!layout().initialized())return;
    const auto id=InventoryNextIdentityLookup(live);const auto found=layout().model().nodes().find(id);
    if(found!=layout().model().nodes().end()&&found->second.container){equipment_dirty=true;layout_current=false;}
} catch (...) {equipment_dirty=true;}
void InventoryNextEquipmentUpdate() noexcept {
    // Actor update owns reconciliation. Render/tooltip paths remain pure reads;
    // defer condition recovery while an ownership transaction is in flight.
    if(!InventoryNextLogicalStorage()||!Actor())return;
    const auto frame=Actor()->inventory().ModifyFrame();
    if((equipment_dirty||frame!=access_inventory_frame||u32(Device.dwTimeGlobal-access_updated_at)>=250)&&
       InventoryNextPickupCanSave()&&InventoryNextLayoutSync()) {
        access_inventory_frame=frame;access_updated_at=Device.dwTimeGlobal;
        const bool refresh=equipment_dirty;equipment_dirty=false;if(refresh)InventoryNextPickupStatus(nullptr);
    }
}
bool InventoryNextActionReady() noexcept {
    return !InventoryNextLogicalStorage()||(InventoryNextPickupCanSave()&&InventoryNextLayoutSync());
}
bool InventoryNextAmmoAccessible(u16 live) noexcept try {
    if(!InventoryNextLogicalStorage())return true;
    if(!equipment_registry_ready||!InventoryNextPickupCanSave()||!layout_current)return false;
    return !ammo_rig_only||inventory_next::equipped_rig_access(layout().model(),InventoryNextIdentityLookup(live));
} catch (...) {return false;}
bool InventoryNextCanUse(u16 live) noexcept try {
    if(!InventoryNextLogicalStorage())return true;
    if(!InventoryNextActionReady())return false;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));if(!item||item->parent_id()!=Actor()->ID())return false;
    if(!medical_rig_only)return true;
    const auto section=item->m_section_id.c_str();
    const std::string kind=pSettings->line_exist(section,"kind")?pSettings->r_string(section,"kind"):"";
    if(kind.empty()||("|"+medical_kinds+"|").find("|"+kind+"|")==std::string::npos)return true;
    if(medical_menu_exception&&CurrentGameUI()&&CurrentGameUI()->GetActorMenu().IsShown())return true;
    const bool allowed=inventory_next::equipped_rig_access(layout().model(),InventoryNextIdentityLookup(live));
    if(!allowed)InventoryNextPickupStatus("No medicine of that type in the equipped rig.");
    return allowed;
} catch (...) {return false;}
u16 InventoryNextQuickPick(const char* section) noexcept try {
    if(!section||!Actor()||!InventoryNextActionReady())return u16(-1);
    for(auto item:Actor()->inventory().m_all)if(item&&item->Useful()&&!xr_strcmp(item->m_section_id.c_str(),section)&&InventoryNextCanUse(item->object_id()))return item->object_id();
    return u16(-1);
} catch (...) {return u16(-1);}
void InventoryNextRigHit(float power,int hit_type) noexcept try {
    if(!InventoryNextLogicalStorage()||!std::isfinite(power)||power<=0||!Actor()||!read_equipment_registry()||!rig_wear_enabled)return;
    switch(hit_type) {
    case ALife::eHitTypeWound:case ALife::eHitTypeFireWound:case ALife::eHitTypeWound_2:
    case ALife::eHitTypeExplosion:case ALife::eHitTypeStrike:case ALife::eHitTypeBurn:break;
    default:return;
    }
    if(u32(Device.dwTimeGlobal-rig_wear_started)>=1000){rig_wear_started=Device.dwTimeGlobal;rig_wear_spent=0;}
    const float room=_max(0.0f,rig_wear_per_second-rig_wear_spent);if(room<=0)return;
    const auto slot=equipment_registry.find("sqa3.rig");
    auto rig=slot?Actor()->inventory().ItemFromCustomSlot(1,slot->id):nullptr;if(!rig)return;
    const float loss=_min(rig->GetCondition(),_min(power*rig_wear_per_hit,room));
    if(loss>0){rig->SetCondition(_max(0.0f,rig->GetCondition()-loss));rig_wear_spent+=loss;}
} catch (...) {Msg("! SQA3 rig wear update failed");}
bool InventoryNextCanCustomEquip(u64 host,u32 slot_id,u16 live,bool restoring) noexcept try {
    using namespace inventory_next;
    if(!InventoryNextLogicalStorage()||!Actor()||!g_pGameLevel||!read_equipment_registry())return false;
    const auto slot=equipment_registry.find(slot_id);
    if(!slot||(!restoring&&!slot->enabled)||(restoring&&!restoring_custom_bindings))return false;
    const auto slot_definition=*slot;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
    if(!item||item->parent_id()!=Actor()->ID())return false;
    StorageCatalog catalog;if(!read_storage_catalog(catalog))return false;
    const auto definition=catalog.definitions.find(item->m_section_id.c_str());
    if(definition==catalog.definitions.end()||!slot_definition.accepts(definition->second.family))return false;
    if(slot_definition.mounted) {
        if(host<100)return false;
        auto owner=smart_cast<CInventoryItem*>(Level().Objects.net_Find(InventoryNextIdentityServer(host)));
        if(!owner||owner==item||owner->parent_id()!=Actor()->ID())return false;
        const auto owner_definition=catalog.definitions.find(owner->m_section_id.c_str());
        if(owner_definition==catalog.definitions.end()||owner_definition->second.family!="rig")return false;
        if(!restoring&&owner->GetCondition()*10000.0f<float(slot_definition.minimum_condition))return false;
    } else if(host!=1)return false;
    if(auto occupied=Actor()->inventory().ItemFromCustomSlot(host,slot_id))if(occupied!=item)return false;
    if(restoring)return true;
    if(!InventoryNextPickupCanSave()||!InventoryNextLayoutSync())return false;
    const auto id=InventoryNextIdentityLookup(live);if(!layout().model().nodes().count(id))return false;
    Command command;command.item=id;command.destination=host;command.session=layout().model().session();command.revision=layout().model().revision();
    if(slot_definition.mounted){command.operation=Operation::Mount;command.mount_slot=slot_id;}
    else {command.operation=Operation::Equip;command.slots=std::uint64_t{1}<<slot_definition.model_bit;}
    return layout().model().preview(command).result.ok;
} catch (...) {return false;}
bool InventoryNextCanCustomSwap(u64 host,u32 slot,u16 incoming) noexcept try {
    if(host!=1||!InventoryNextLogicalStorage()||!Actor()||!InventoryNextActionReady())return false;
    const auto definition=equipment_registry.find(slot);if(!definition||!definition->enabled||definition->mounted)return false;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(incoming));
    auto outgoing=Actor()->inventory().ItemFromCustomSlot(host,slot);
    if(!item||!outgoing||item==outgoing||item->parent_id()!=Actor()->ID()||item->CurrPlace()!=eItemPlaceRuck)return false;
    const auto id=InventoryNextIdentityLookup(incoming);const auto found=layout().model().nodes().find(id);
    if(found==layout().model().nodes().end()||!found->second.attributes.count("storage_family")||!definition->accepts(found->second.attributes.at("storage_family")))return false;
    auto candidate=layout().model();return candidate.swap_equipment(id,InventoryNextIdentityLookup(outgoing->object_id()),candidate.revision(),candidate.session()).ok;
} catch (...) {return false;}
bool InventoryNextCustomEquip(u16 live,u64 host,const char* key) noexcept try {
    if(!key||!read_equipment_registry()||!Actor())return false;
    const auto slot=equipment_registry.find(std::string(key));
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
    if(slot&&item)if(auto outgoing=Actor()->inventory().ItemFromCustomSlot(host,slot->id))if(outgoing!=item){
        if(!InventoryNextCanCustomSwap(host,slot->id,live)){InventoryNextPickupStatus("The equipped item does not fit in the vacated pocket position.");return false;}
        auto candidate=layout().model();const auto plan=candidate.swap_equipment(InventoryNextIdentityLookup(live),InventoryNextIdentityLookup(outgoing->object_id()),candidate.revision(),candidate.session());
        if(!plan||!Actor()->inventory().SwapCustomSlot(host,slot->id,item))return false;
        if(!layout().restore(candidate.serialize())||!InventoryNextLayoutSync()){pickup_fault=true;return false;}
        InventoryNextPickupStatus(nullptr);return true;
    }
    const bool result=slot&&item&&Actor()->inventory().CustomSlot(host,slot->id,item);
    if(!result)InventoryNextPickupStatus("Equipment slot unavailable, occupied, damaged, or incompatible.");
    else if(!InventoryNextLayoutSync())return false;
    return result;
} catch (...) {return false;}
u16 InventoryNextCustomItem(u64 host,const char* key) noexcept try {
    if(!Actor()||!key||!read_equipment_registry())return u16(-1);
    const auto slot=equipment_registry.find(std::string(key));
    auto item=slot?Actor()->inventory().ItemFromCustomSlot(host,slot->id):nullptr;
    return item?item->object_id():u16(-1);
} catch (...) {return u16(-1);}
std::vector<InventoryNextEquipmentAction> InventoryNextEquipmentActions(u16 live) noexcept try {
    std::vector<InventoryNextEquipmentAction> actions;
    if(!InventoryNextLogicalStorage()||!Actor()||!InventoryNextLayoutSync())return actions;
    auto native=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
    if(native&&native->parent_id()!=Actor()->ID()){
        inventory_next::StorageCatalog catalog;if(!read_storage_catalog(catalog))return actions;
        const auto found=catalog.definitions.find(native->m_section_id.c_str());if(found==catalog.definitions.end())return actions;
        const auto identity=InventoryNextIdentity(live);if(!identity)return actions;
        for(const auto& entry:equipment_registry.slots()){
            const auto& slot=entry.second;if(!slot.enabled||!slot.accepts(found->second.family))continue;
            const auto rig=slot.mounted?InventoryNextCustomItem(1,"sqa3.rig"):u16(-1);
            if(slot.mounted&&rig==u16(-1))continue;
            actions.push_back({slot.key,"Equip "+slot.label,slot.mounted?InventoryNextIdentityLookup(rig):u64{1},identity,live,false,false,true});
        }
        return actions;
    }
    const auto id=InventoryNextIdentityLookup(live);const auto& nodes=layout().model().nodes();
    const auto found=nodes.find(id);if(found==nodes.end())return actions;
    const auto family=found->second.attributes.find("storage_family");if(family==found->second.attributes.end())return actions;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));if(!item||item->parent_id()!=Actor()->ID())return actions;
    u64 current_host=0;u32 current_slot=0;
    if(Actor()->inventory().CustomSlotFor(item,current_host,current_slot))actions.push_back({{},"Unequip / detach",current_host,id,live,true,false});
    else for(const auto& entry:equipment_registry.slots()) {
        const auto& slot=entry.second;if(!slot.enabled||!slot.accepts(family->second))continue;
        if(!slot.mounted)actions.push_back({slot.key,"Equip "+slot.label,1,id,live,false,false});
        else {
            const auto rig=InventoryNextCustomItem(1,"sqa3.rig");
            if(rig!=u16(-1))actions.push_back({slot.key,"Fit "+slot.label,InventoryNextIdentityLookup(rig),id,live,false,false});
        }
    }
    if(family->second=="rig")for(const auto& entry:equipment_registry.slots())if(entry.second.mounted){
        const auto pouch=Actor()->inventory().ItemFromCustomSlot(id,entry.first);if(!pouch)continue;
        const auto pouch_id=InventoryNextIdentityLookup(pouch->object_id());
        actions.push_back({entry.second.key,"Open "+entry.second.label,id,pouch_id,pouch->object_id(),false,true});
        actions.push_back({entry.second.key,"Detach "+entry.second.label,id,pouch_id,pouch->object_id(),true,false});
    }
    return actions;
} catch (...) {return {};}
bool InventoryNextCustomUnequip(u16 live,int x,int y,u64 view) noexcept try {
    using namespace inventory_next;
    if(!InventoryNextLogicalStorage()||!InventoryNextPickupCanSave()||!InventoryNextLayoutSync())return false;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));u64 host=0;u32 slot=0;
    if(!item||!Actor()->inventory().CustomSlotFor(item,host,slot))return false;
    const auto id=InventoryNextIdentityLookup(live);const auto& model=layout().model();
    if(!model.nodes().count(id))return false;
    if(model.nodes().at(id).location==Location::Mounted)for(const auto& node:model.nodes())if(node.second.parent==id){InventoryNextPickupStatus("Empty this pouch before detaching it.");return false;}
    inventory_next::Position position;
    if(x>=0&&y>=0) {
        if(!InventoryNextNativeDragCurrent(live))return false;
        Id destination=0;const auto& selected=view?make_storage_view(view):storage_view;
        if(!selected.destination(x,y,destination,position)||destination!=1){native_drag().cancel();return false;}
        const auto proposal=native_drag().release(model,destination,position.x,position.y,false,{});
        if(!proposal.result){InventoryNextPickupStatus("This item does not fit in that pocket position.");return false;}
        position=proposal.moves.front().position;
    } else {
        const auto placement=find_placement(model,id,{1});
        if(!placement.result){InventoryNextPickupStatus("No room to put this item in your pockets.");return false;}
        position=placement.moves.front().position;
    }
    if(!Actor()->inventory().Ruck(item,true)||!InventoryNextLayoutSync())return false;
    const auto result=layout().move({id,1,position,layout().model().revision(),layout().model().session()});
    InventoryNextCustomChanged();return result.ok;
} catch (...) {return false;}
bool InventoryNextManualUnequip(u16 live,int x,int y,u64 view) noexcept try {
    using namespace inventory_next;
    if(!InventoryNextLogicalStorage()||!Actor()||!InventoryNextActionReady())return false;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
    if(!item||item->parent_id()!=Actor()->ID())return false;
    if(item->CurrPlace()==eItemPlaceCustom)return InventoryNextCustomUnequip(live,x,y,view);
    if((item->CurrPlace()!=eItemPlaceSlot&&item->CurrPlace()!=eItemPlaceBelt)||!Actor()->inventory().CanPutInRuck(item))return false;
    const auto id=InventoryNextIdentityLookup(live);const auto& model=layout().model();
    if(!model.nodes().count(id))return false;
    Id destination=1;inventory_next::Position position;
    if(x>=0&&y>=0){
        if(!InventoryNextNativeDragCurrent(live))return false;
        const auto& selected=view?make_storage_view(view):storage_view;
        if(!selected.destination(x,y,destination,position)){native_drag().cancel();return false;}
        const auto proposal=native_drag().release(model,destination,position.x,position.y,false,{});
        if(!proposal.result){InventoryNextPickupStatus("This item does not fit at that position.");return false;}
        position=proposal.moves.front().position;
    } else {
        std::vector<Id> destinations{1};
        for(const auto& pair:model.nodes())if(pair.second.kind==Kind::Item&&pair.second.container&&pair.first!=id&&
            pair.second.location!=Location::Overflow&&model.storage_active(pair.first))destinations.push_back(pair.first);
        const auto placement=find_placement(model,id,destinations);
        if(!placement.result){InventoryNextPickupStatus("No legal space to unequip this item.");return false;}
        destination=placement.moves.front().destination;position=placement.moves.front().position;
    }
    if(!Actor()->inventory().Ruck(item,true)||!InventoryNextLayoutSync())return false;
    const auto result=layout().move({id,destination,position,layout().model().revision(),layout().model().session()});
    if(!result){pickup_fault=true;InventoryNextPickupStatus("Equipment changed during unequip. Saving blocked.");return false;}
    InventoryNextCustomChanged();return true;
} catch (...) {return false;}
bool InventoryNextPrepareDismantle(u16 live) noexcept try {
    using namespace inventory_next;
    if(!InventoryNextLogicalStorage())return true;
    if(!Actor()||!InventoryNextActionReady())return false;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
    if(!item||item->parent_id()!=Actor()->ID()||item->IsQuestItem())return false;
    const auto id=InventoryNextIdentityLookup(live);const auto found=layout().model().nodes().find(id);
    if(found==layout().model().nodes().end())return false;
    if(!found->second.container)return true;
    xr_vector<PIItem> pouches;auto candidate=layout().model();
    for(const auto& entry:candidate.nodes())if(entry.second.parent==id){
        if(entry.second.location!=Location::Mounted){InventoryNextPickupStatus("Empty this container before dismantling it.");return false;}
        for(const auto& child:candidate.nodes())if(child.second.parent==entry.first){InventoryNextPickupStatus("Empty the mounted pouches before dismantling this rig.");return false;}
        auto pouch=smart_cast<CInventoryItem*>(Level().Objects.net_Find(InventoryNextIdentityServer(entry.first)));
        if(!pouch||pouch->parent_id()!=Actor()->ID())return false;pouches.push_back(pouch);
    }
    for(auto pouch:pouches){const auto result=candidate.recover_owned_overflow(InventoryNextIdentityLookup(pouch->object_id()),1,candidate.revision(),candidate.session());if(!result)return false;}
    for(auto pouch:pouches){
        const auto placement=find_placement(candidate,InventoryNextIdentityLookup(pouch->object_id()),{1});
        if(placement.result&&!candidate.relocate(placement.moves))return false;
    }
    const auto serialized=candidate.serialize();
    if(!dismantling_items.insert(id).second)return false;
    if(!pouches.empty()&&!Actor()->inventory().DetachCustomMounts(id,pouches)){dismantling_items.erase(id);return false;}
    if(!layout().restore(serialized)||!InventoryNextLayoutSync()){pickup_fault=true;return false;}
    Msg("* SQA3 dismantle prepared live=%u returned_pouches=%u",live,static_cast<unsigned>(pouches.size()));return true;
} catch (...) {return false;}
bool InventoryNextLayoutSync() noexcept try {
    layout_current=false;
    if(!InventoryNextEnabled()||!g_pGameLevel||!Actor())return true;
    using namespace inventory_next;
    const bool logical=InventoryNextLogicalStorage();
    StorageCatalog catalog;
    if(logical&&!read_storage_catalog(catalog))return false;
    if(logical&&!read_equipment_registry())return false;
    if(logical&&!pending_custom_bindings.empty()) {
        restoring_custom_bindings=true;
        for(unsigned pass=0;pass<2;++pass)for(auto it=pending_custom_bindings.begin();it!=pending_custom_bindings.end();) {
            const auto slot=equipment_registry.find(it->slot);
            if(!slot){Msg("! SQA3 retired custom slot=%u; item retained in recovery",it->slot);it=pending_custom_bindings.erase(it);continue;}
            if(slot->mounted!=(pass==1)){++it;continue;}
            auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(InventoryNextIdentityServer(it->item)));
            if(!item){++it;continue;}
            if(item->parent_id()==Actor()->ID()&&!Actor()->inventory().CustomSlot(it->host,it->slot,item,true)){restoring_custom_bindings=false;return false;}
            it=pending_custom_bindings.erase(it);
        }
        restoring_custom_bindings=false;
        if(!pending_custom_bindings.empty())return false;
    }
    std::vector<Node> nodes;
        for (auto* item : Actor()->inventory().m_all) {
            if (!item || nodes.size()>=4094) { Msg("! SQA3 probe: invalid/count-limited inventory"); return false; }
            bool departing=false;
            if(world_transfer&&world_transfer->committed&&!world_transfer->acquire)
                for(const auto& record:world_transfer->records)if(record.live==item->object_id()){departing=true;break;}
            if(world_member&&world_member->committed&&world_member->actor_source&&world_member->live==item->object_id())departing=true;
            if(departing)continue;
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
            native_instance_state(n,item);
            if(logical&&!catalog.apply(item->m_section_id.c_str(),n)&&smart_cast<CBackpack*>(item)) {
                const std::string section=item->m_section_id.c_str();
                const std::string kind=pSettings->line_exist(section.c_str(),"kind")?pSettings->r_string(section.c_str(),"kind"):"";
                const auto& shape=catalog.resolve(section,kind);
                n.container=true;n.grid=shape.grid;n.cells=shape.cells;
            }
            if(logical)apply_custom_binding(n,item);
            nodes.push_back(n);
        }
        // Server-acknowledged ownership already reserves storage. Keep the
        // admitted facts until CInventory::Take observes the actual client item.
        for(const auto& pending:pending_pickups) {
            bool present=false;for(const auto& n:nodes)if(n.id==pending.second.id){present=true;break;}
            if(!present){auto n=pending.second;if(n.location==Location::Grid){n.parent=1;n.location=Location::Overflow;n.position={};}nodes.push_back(std::move(n));}
        }
        auto storage=logical?catalog.pockets:ActorStorage{};storage.logical=logical;
        const auto result=layout().reconcile(std::move(nodes),storage);
        if(!result)Msg("! SQA3 native layout sync: %s",result.error.c_str());
        layout_current=result.ok;storage_views.clear();
        if(layout_current&&logical)layout_current=storage_view.build(layout().model(),view_options()).ok;
        return layout_current;
} catch (...) {Msg("! SQA3 native layout sync exception");return false;}
void InventoryNextLayoutReset() noexcept try {source_footprints.clear();dismantling_items.clear();equipment_registry_ready=false;storage_catalog_cache.reset();pending_custom_bindings.clear();restoring_custom_bindings=false;collapsed_storage.clear();storage_views.clear();stash_markers.clear();layout_current=false;native_drag().cancel();ground_provision.reset();world_transfer.reset();world_member.reset();purchase.reset();pickup_transaction.reset();pickup_registry.reset();pickup_world_candidate.reset();pickup_world_before.clear();pending_pickups.clear();pickup_fault=false;layout().reset();world_storage()=inventory_next::WorldSubtrees{};} catch (...) {CHECK_OR_EXIT(false,"SQA3 layout reset failed");}
void InventoryNextLayoutSave(IWriter& stream) noexcept try {
    if(!layout().initialized())return;
    const auto bytes=layout().model().serialize();stream.open_chunk(layout_chunk);
    stream.w(bytes.data(),static_cast<u32>(bytes.size()));stream.close_chunk();
    if(InventoryNextLogicalStorage()) {
        const auto world_bytes=world_storage().serialize();stream.open_chunk(world_storage_chunk);
        stream.w(world_bytes.data(),static_cast<u32>(world_bytes.size()));stream.close_chunk();
    }
} catch (...) {CHECK_OR_EXIT(false,"SQA3 layout save failed");}
void InventoryNextLayoutLoad(IReader& stream) noexcept try {
    source_footprints.clear();dismantling_items.clear();
    equipment_registry_ready=false;storage_catalog_cache.reset();
    pending_custom_bindings.clear();restoring_custom_bindings=false;
    collapsed_storage.clear();storage_views.clear();stash_markers.clear();layout_current=false;native_drag().cancel();world_transfer.reset();world_member.reset();purchase.reset();pickup_transaction.reset();pickup_registry.reset();pickup_world_candidate.reset();pickup_world_before.clear();pending_pickups.clear();pickup_fault=false;layout().reset();world_storage()=inventory_next::WorldSubtrees{};auto chunk=stream.open_chunk(layout_chunk);
    if(!chunk) {
        auto orphan=stream.open_chunk(world_storage_chunk);const bool has_world=orphan!=nullptr;if(orphan)orphan->close();
        CHECK_OR_EXIT(!has_world,"SQA3 world storage missing actor layout");return;
    }
    CHECK_OR_EXIT(InventoryNextEnabled()&&chunk->length()<=2*1024*1024,"Invalid SQA3 layout mode/size");
    const std::string bytes(reinterpret_cast<const char*>(chunk->pointer()),chunk->length());chunk->close();
    CHECK_OR_EXIT(layout().restore(bytes).ok,"Invalid SQA3 layout data");
    queue_custom_bindings();
    auto world_chunk=stream.open_chunk(world_storage_chunk);
    if(world_chunk) {
        CHECK_OR_EXIT(world_chunk->length()<=2*1024*1024,"Invalid SQA3 world storage size");
        const std::string world_bytes(reinterpret_cast<const char*>(world_chunk->pointer()),world_chunk->length());world_chunk->close();
        CHECK_OR_EXIT(world_storage().restore(world_bytes).ok,"Invalid SQA3 world storage data");
        CHECK_OR_EXIT(world_storage().records().empty()||InventoryNextWorldStorageEnabled(),"SQA3 world storage requires world storage mode");
        for(const auto& pair:world_storage().records())
            CHECK_OR_EXIT(world_storage().stash_name(pair.first).empty()||InventoryNextStashEnabled(),"SQA3 named stashes require stash storage mode");
    }
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
    if(!pending_pickups.empty()||pickup_transaction||purchase||world_transfer||world_member)return false;
    if(!InventoryNextLayoutSync())return false;
    const auto id=InventoryNextIdentity(live);
    if(!id||!layout().model().nodes().count(id))return true;
    for(const auto& pair:layout().model().nodes())if(pair.second.parent==id) {
        Msg("! SQA3 loaded storage release refused live=%u; subtree transfer not ready",live);return false;
    }
    return true;
} catch (...) {return false;}
bool InventoryNextCanBulkRelease(u16 live) noexcept try {
    if(!InventoryNextLogicalStorage())return true;
    if(!InventoryNextCanRelease(live))return false;
    const auto id=InventoryNextIdentity(live);
    const auto found=layout().model().nodes().find(id);
    // Bulk release must not flatten a loaded logical container into unrelated
    // world items. Manual moves of an individual contained item remain allowed.
    if(found!=layout().model().nodes().end()&&found->second.parent!=1) {
        Msg("! SQA3 nested bulk release refused live=%u; containing storage retained",live);
        return false;
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
            if(InventoryNextWorldStorageEnabled()) {
                string_path world_path;FS.update_path(world_path,"$app_data_root$","sqa3-world-storage.inv");
                const auto world_bytes=world_storage().serialize();auto file=FS.w_open(world_path);const bool written=file!=nullptr;
                if(file){file->w(world_bytes.data(),static_cast<u32>(world_bytes.size()));FS.w_close(file);}
                Msg("* SQA3 world storage records=%u export=%s",static_cast<unsigned>(world_storage().records().size()),written?"written":"failed");
                for(const auto& record:world_storage().records())
                    Msg("* SQA3 world bag id=%llu custodian=%llu items=%u grams=%lld",record.first,record.second.custodian,static_cast<unsigned>(record.second.tree.nodes().size()-2),static_cast<long long>(record.second.tree.subtree_mass(record.first)));
            }
        }
    } catch(const std::exception& e) {
        Msg("! SQA3 probe failed: %s",e.what());
    } catch (...) { Msg("! SQA3 probe failed: unknown core exception"); }
}

bool InventoryNextNativeFootprint(u16 live,int& width,int& height) noexcept try {
    if(!InventoryNextEnabled()||!layout_current||!layout().initialized())return false;
    const auto id=InventoryNextIdentityLookup(live);
    if(!id)return false;
    inventory_next::Size size;
    if(layout().model().nodes().count(id))size=layout().model().footprint(id);
    else {
        bool found_world=false;
        for(const auto& record:world_storage().records())if(record.second.tree.nodes().count(id)){
            size=record.second.tree.footprint(id);found_world=true;break;
        }
        if(!found_world){const auto found=source_footprints.find(id);if(found==source_footprints.end())return false;size=found->second;}
    }
    width=size.w;height=size.h;return size.w>0&&size.h>0;
} catch (...) {return false;}
bool InventoryNextNativePosition(u16 live,int& x,int& y,bool& rotated,u64 view) noexcept try {
    if(!InventoryNextEnabled()||!layout_current||!layout().initialized())return false;
    const auto id=InventoryNextIdentity(live);
    const auto record=world_storage().records().find(view);
    const auto& model=record==world_storage().records().end()?layout().model():record->second.tree;
    const auto found=model.nodes().find(id);
    if(found==model.nodes().end())return false;
    if(InventoryNextLogicalStorage()) {
        const auto& selected=view?make_storage_view(view):storage_view;
        inventory_next::Position p;if(!selected.position(found->second,p))return false;
        x=p.x;y=p.y;rotated=p.rotated;return true;
    }
    if(found->second.location!=inventory_next::Location::Grid)return false;
    x=found->second.position.x;y=found->second.position.y;rotated=found->second.position.rotated;return true;
} catch (...) {return false;}
bool InventoryNextStorageExtent(u64 view,int& width,int& height) noexcept try {
    if(!layout_current||!InventoryNextLogicalStorage())return false;
    const auto& selected=view?make_storage_view(view):storage_view;
    width=selected.size.w;height=selected.size.h;return true;
} catch (...) {return false;}
int InventoryNextStorageCell(int x,int y,u64 view) noexcept try {
    if(!layout_current||!InventoryNextLogicalStorage())return 0;
    inventory_next::Id owner=0;inventory_next::Position local;
    const auto& selected=view?make_storage_view(view):storage_view;
    if(selected.region(x,y))return 1;
    return !selected.recovery.empty()&&y>=selected.recovery_row?2:0;
} catch (...) {return 0;}
float InventoryNextStorageInset(int y,u64 view) noexcept try {
    if(!layout_current||!InventoryNextFieldKitEnabled())return 0.0f;
    const auto& selected=view?make_storage_view(view):storage_view;return selected.row_inset(y);
} catch (...) {return 0.0f;}
unsigned InventoryNextStorageRegion(int x,int y,u64 view) noexcept try {
    if(!layout_current||!InventoryNextFieldKitEnabled())return 0;
    const auto& selected=view?make_storage_view(view):storage_view;
    return selected.region(x,y);
} catch (...) {return 0;}
void InventoryNextStorageView(CUIDragDropListEx* list,u64 view) noexcept try {
    if(!InventoryNextLogicalStorage()||!layout_current||!list)return;
    const auto& selected=view?make_storage_view(view):storage_view;
    const auto record=world_storage().records().find(view);
    const auto& model=record==world_storage().records().end()?layout().model():record->second.tree;
    const int spacing=2;
    const int body=inventory_next::fitted_cell_body(list->SqaGridContentWidth(),selected.size.w,spacing,list->CellSize().x);
    if(body<1)return;
    if(list->CellSize().x!=body||list->CellSize().y!=body)list->SetCellSize(Ivector2().set(body,body));
    if(list->CellsSpacing().x!=spacing||list->CellsSpacing().y!=spacing)list->SetCellsSpacing(Ivector2().set(spacing,spacing));
    list->sqaViewOwner=view;
    list->SetStartCellsCapacity(Ivector2().set(selected.size.w,selected.size.h));
    if(InventoryNextFieldKitEnabled())list->CenterStorageContent();
    list->SetAutoGrow(false);list->SetGrouping(false);
    for(const auto& band:selected.bands) {
        if(!band.header)continue;
        std::string label="Pockets";
        if(band.owner!=1) {
            const auto& n=model.nodes().at(band.owner);
            const auto& section=n.attributes.at("native_section");
            label=pSettings->line_exist(section.c_str(),"inv_name")?
                CStringTable().translate(pSettings->r_string(section.c_str(),"inv_name")).c_str():"Backpack";
            if(!InventoryNextFieldKitEnabled())label+=n.location==inventory_next::Location::Equipment?" (equipped)":" (carried)";
        }
        if(!InventoryNextFieldKitEnabled())label+="  "+std::to_string(band.grid.w)+" x "+std::to_string(band.grid.h);
        inventory_next::Mass contents=0;
        for(const auto& pair:model.nodes())if(pair.second.parent==band.owner&&
            (pair.second.location==inventory_next::Location::Grid||pair.second.location==inventory_next::Location::Mounted))
            contents+=model.subtree_mass(pair.first);
        list->AddStorageHeader(band.row-1,label.c_str(),0xffd8dacb,band.owner,contents,band.collapsed);
    }
    if(!selected.recovery.empty()){
        inventory_next::Mass contents=0;
        for(const auto& item:selected.recovery)contents+=model.subtree_mass(item.first);
        list->AddStorageHeader(selected.recovery_row-1,"Recovery",0xffff9d70,0,contents);
    }
} catch (...) {Msg("! SQA3 storage view failed");}
bool InventoryNextNativeDrop(u16 live,int x,int y,u64 view) noexcept try {
    if(!InventoryNextEnabled()||!InventoryNextLayoutSync())return false;
    const auto id=InventoryNextIdentity(live);const auto& model=layout().model();
    const auto found=model.nodes().find(id);if(found==model.nodes().end())return false;
    if(!InventoryNextLogicalStorage()&&found->second.location!=inventory_next::Location::Grid)return false;
    if(native_drag().item()!=id){Msg("! SQA3 native drag missing source token");native_drag().cancel();return false;}
    inventory_next::Id owner=1;inventory_next::Position local{x,y,false};
    const auto& selected=view?make_storage_view(view):storage_view;
    if(InventoryNextLogicalStorage()&&!selected.destination(x,y,owner,local)) {native_drag().cancel();return false;}
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
    return !pickup_fault&&!pickup_transaction&&!pickup_world_candidate&&!purchase&&!world_transfer&&!world_member&&!ground_provision&&pending_pickups.empty()&&dismantling_items.empty();
}
void InventoryNextPickupForget(u16 live) noexcept try {
    if(CurrentGameUI())CurrentGameUI()->GetActorMenu().SqaForgetLoot(live);
    const auto identity=InventoryNextIdentityLookup(live);
    dismantling_items.erase(identity);
    source_footprints.erase(identity);
    for(const auto& record:world_storage().records())
        if(record.second.custodian==identity||record.second.tree.nodes().count(identity))pickup_fault=true;
    if(world_transfer)for(const auto& record:world_transfer->records)if(record.live==live)pickup_fault=true;
    if(world_member&&world_member->live==live)pickup_fault=true;
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
u64 detached_leaf_shell(u16 source,u16 live) {
    if(source==0xffff)return 0;
    const auto source_identity=InventoryNextIdentity(source),item_identity=InventoryNextIdentity(live);
    if(!source_identity||!item_identity)return 0;
    for(const auto& record:world_storage().records())if(record.second.custodian==source_identity&&record.first!=item_identity&&
        record.second.tree.nodes().count(item_identity)){
        if(record.second.tree.nodes().at(item_identity).parent!=record.first)return 0;
        for(const auto& pair:record.second.tree.nodes())if(pair.second.parent==item_identity)return 0;
        return record.first;
    }
    return 0;
}
}
bool InventoryNextPickupPrepare(u16 owner,u16 live,u16 source,u16 slot,u32 custom_slot,u64 custom_host,u64 exact_destination,int exact_x,int exact_y,u64 expected_logical_owner) noexcept try {
    if(!InventoryNextStrictPickup()||!Actor()||owner!=Actor()->ID())return true;
    using namespace inventory_next;
    if(pickup_fault||pickup_transaction||purchase||world_transfer||!InventoryNextLayoutSync())return false;
    const auto detached_shell=detached_leaf_shell(source,live);
    if((detached_shell||expected_logical_owner)&&detached_shell!=expected_logical_owner)return false;
    if(!InventoryNextWorldCanRelease(live)&&!detached_shell)return false;
    auto object=Level().Objects.net_Find(live);
    auto item=smart_cast<CInventoryItem*>(object);
    const auto server=ai().alife().objects().object(live,true);
    if(!item||!server||server->ID_Parent!=source)return false;
    const auto actual_parent=item->object().H_Parent();
    if(source==0xffff?actual_parent!=nullptr:(!actual_parent||actual_parent->ID()!=source))return false;
    if(source!=0xffff&&!InventoryNextStrictOwnedPickup())return false;
    if(!Actor()->inventory().CanTakeItem(item)){InventoryNextPickupStatus("Item cannot be taken. Source unchanged.");return false;}
    if(exact_destination&&(slot!=0xffff||custom_slot||exact_x<0||exact_y<0||exact_x>=64||exact_y>=64))return false;
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
    std::unique_ptr<WorldSubtrees> world_candidate;
    if(detached_shell){
        world_candidate=std::make_unique<WorldSubtrees>();
        const auto result=world_storage().without_leaf(detached_shell,InventoryNextIdentity(source),id,*world_candidate);
        if(!result)return false;
    }
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
    native_instance_state(n,item);
    if(slot!=0xffff) {
        n.wear_slots=std::uint64_t{1}<<slot;
        n.attributes["native_place"]="1";n.attributes["native_slot"]=std::to_string(slot);
    }
    StorageCatalog catalog;if(!read_storage_catalog(catalog))return false;
    if(!catalog.apply(item->m_section_id.c_str(),n)&&smart_cast<CBackpack*>(item)) {
        const std::string section=item->m_section_id.c_str();
        const std::string kind=pSettings->line_exist(section.c_str(),"kind")?pSettings->r_string(section.c_str(),"kind"):"";
        const auto& storage=catalog.resolve(section,kind);n.container=true;n.grid=storage.grid;n.cells=storage.cells;
    }
    const EquipmentSlotDefinition* custom=nullptr;
    if(custom_slot) {
        custom=equipment_registry.find(custom_slot);
        const auto family=n.attributes.find("storage_family");
        if(slot!=0xffff||!custom||!custom->enabled||family==n.attributes.end()||!custom->accepts(family->second)||Actor()->inventory().ItemFromCustomSlot(custom_host,custom_slot))return false;
        if(custom->mounted) {
            const auto host=layout().model().nodes().find(custom_host);
            if(host==layout().model().nodes().end()||!host->second.attributes.count("storage_family")||host->second.attributes.at("storage_family")!="rig"||host->second.condition<custom->minimum_condition)return false;
            n.pouch=true;
        } else {
            if(custom_host!=1)return false;
            n.wear_slots=std::uint64_t{1}<<custom->model_bit;
        }
        n.attributes["native_place"]="4";n.attributes["native_slot"]="0";
        n.attributes["native_custom_slot"]=std::to_string(custom_slot);n.attributes["native_custom_host"]=std::to_string(custom_host);n.attributes["native_custom_key"]=custom->key;
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
    const auto result=transaction->prepare(layout().model(),n,destinations,source==0xffff,
        custom&&!custom->mounted?n.wear_slots:(slot==0xffff?0:n.wear_slots),custom&&custom->mounted?custom_host:0,custom&&custom->mounted?custom_slot:0,
        exact_destination,{exact_x,exact_y,false});
    if(!result){Msg("! SQA3 %s pickup refused live=%u reason=%s; source retained",source==0xffff?"ground":"owned",live,result.error.c_str());InventoryNextPickupStatus("No space. Item stayed at source.");return false;}
    pickup_registry=std::move(registry);pickup_transaction=std::move(transaction);pickup_source=source;
    if(world_candidate){pickup_world_before=world_storage().serialize();pickup_world_candidate=std::move(world_candidate);}
    if(slot!=0xffff)Msg("* SQA3 direct equip reserved live=%u slot=%u",live,slot);
    Msg("* SQA3 %s pickup reserved live=%u; save blocked=%d",source==0xffff?"ground":"owned",live,!InventoryNextPickupCanSave());
    return true;
} catch (...) {Msg("! SQA3 ground pickup prepare exception; source retained");return false;}
void InventoryNextPickupFinish(u16 owner,u16 live,bool accepted) noexcept try {
    if(!InventoryNextStrictPickup()||!Actor()||owner!=Actor()->ID()||!pickup_transaction)return;
    const auto server=ai().alife().objects().object(live,true);
    if(pickup_world_candidate&&world_storage().serialize()!=pickup_world_before){pickup_fault=true;return;}
    const auto result=pickup_transaction->acknowledge(server?server->ID_Parent:0xffff,accepted);
    if(accepted&&result) {
        const auto id=InventoryNextIdentity(live);
        const auto admitted=pickup_transaction->model().nodes().at(id);
        // Restore only the acknowledged actor forest, never a rejected source.
        const auto restored=layout().restore(pickup_transaction->model().serialize());
        if(!restored){pickup_fault=true;Msg("! SQA3 pickup publish failed: %s",restored.error.c_str());return;}
        if(pickup_world_candidate)world_storage()=std::move(*pickup_world_candidate);
        pending_pickups.emplace(live,admitted);
        InventoryNextPickupStatus("");
        if(InventoryNextIdentityCanSave()){pickup_fault=true;Msg("! SQA3 pending pickup incorrectly allowed save");return;}
        Msg("* SQA3 %s pickup server accepted live=%u owner=%llu cell=%d,%d",pickup_source==0xffff?"ground":"owned",live,admitted.parent,admitted.position.x,admitted.position.y);
    } else if(accepted||result.error!="native_rejected") {
        pickup_fault=true;Msg("! SQA3 pickup acknowledgement failed: %s",result.error.c_str());return;
    }
    pickup_transaction.reset();pickup_registry.reset();pickup_world_candidate.reset();pickup_world_before.clear();
} catch (...) {pickup_fault=true;Msg("! SQA3 pickup acknowledgement exception; saves blocked");}
void InventoryNextPickupTaken(u16 live) noexcept try {
    if(!pending_pickups.count(live))return;
    const auto expected=pending_pickups.at(live);
    if(expected.attributes.count("native_custom_slot")) {
        auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
        const auto host=std::stoull(expected.attributes.at("native_custom_host"));
        const auto slot=static_cast<u32>(std::stoul(expected.attributes.at("native_custom_slot")));
        const auto definition=equipment_registry.find(slot);
        if(!item||!definition||!definition->enabled){pickup_fault=true;return;}
        if(definition->mounted){auto owner=layout().model().nodes().find(host);if(owner==layout().model().nodes().end()||owner->second.condition<definition->minimum_condition){pickup_fault=true;return;}}
        restoring_custom_bindings=true;const bool bound=Actor()->inventory().CustomSlot(host,slot,item,true);restoring_custom_bindings=false;
        if(!bound){pickup_fault=true;return;}
    }
    pending_pickups.erase(live);
    if(!InventoryNextLayoutSync()){pickup_fault=true;return;}
    const auto id=InventoryNextIdentity(live);const auto found=layout().model().nodes().find(id);
    if(found==layout().model().nodes().end()||found->second.location==inventory_next::Location::Overflow||
       (expected.location==inventory_next::Location::Equipment&&
        (found->second.location!=expected.location||found->second.equipped_slots!=expected.equipped_slots))||
       (expected.location==inventory_next::Location::Mounted&&
        (found->second.location!=expected.location||found->second.parent!=expected.parent||found->second.mounted_slot!=expected.mounted_slot))) {
        pickup_fault=true;Msg("! SQA3 ordinary pickup lost reserved placement; saves blocked");return;
    }
    InventoryNextPickupStatus(nullptr);
    Msg("* SQA3 pickup client acknowledged live=%u; pending=%u save_ready=%d",live,static_cast<unsigned>(pending_pickups.size()),InventoryNextPickupCanSave());
} catch (...) {restoring_custom_bindings=false;pickup_fault=true;}

bool InventoryNextPickupCustomPending(u16 owner,u16 live) noexcept {
    const auto found=pending_pickups.find(live);
    return Actor()&&owner==Actor()->ID()&&found!=pending_pickups.end()&&found->second.attributes.count("native_custom_slot");
}
u16 InventoryNextPickupSlot(u16 owner,u16 live) noexcept try {
    if(!Actor()||owner!=Actor()->ID())return 0xffff;
    const auto pending=pending_pickups.find(live);
    if(pending==pending_pickups.end()||pending->second.location!=inventory_next::Location::Equipment)return 0xffff;
    if(pending->second.attributes.count("native_custom_slot"))return 0xffff;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
    for(u16 slot=1;slot<48;++slot)if(pending->second.equipped_slots==(std::uint64_t{1}<<slot)) {
        if(item&&slot<=Actor()->inventory().LastSlot()&&pickup_slot_compatible(item,slot)&&Actor()->inventory().CanPutInSlot(item,slot))return slot;
        break;
    }
    pickup_fault=true;InventoryNextPickupStatus("Equipment changed during transfer. Saving blocked.");return 0xffff;
} catch (...) {pickup_fault=true;return 0xffff;}


bool InventoryNextStrictOwnedPickup() noexcept {
    return InventoryNextLogicalStorage()&&(strstr(Core.Params,"-sqa3-strict-owned-pickup")!=nullptr||InventoryNextStrictTradePickup()||InventoryNextWorldStorageEnabled());
}
void InventoryNextPickupStatus(const char* text) noexcept try {
    if(!CurrentGameUI())return;
    auto& menu=CurrentGameUI()->GetActorMenu();menu.m_sqaRefresh=true;menu.m_sqaRetryAfter=Device.dwTimeGlobal+250;
    if(text&&menu.m_sqaStatus)menu.m_sqaStatus->TextItemControl()->SetText(text);
} catch (...) {Msg("! SQA3 acquisition status refresh failed");}
bool InventoryNextRequestOwnedPickup(u16 source,u16 owner,u16 live,u16 slot,u32 custom_slot,u64 custom_host,u64 exact_destination,int exact_x,int exact_y,u64 expected_logical_owner) noexcept try {
    if(!InventoryNextStrictOwnedPickup()||!Actor()||owner!=Actor()->ID()||source==owner||(source==0xffff&&!custom_slot))return false;
    if(exact_destination&&(slot!=0xffff||custom_slot||exact_x<0||exact_y<0||exact_x>=64||exact_y>=64))return false;
    if(!InventoryNextWorldCanRelease(live)&&!detached_leaf_shell(source,live)) {
        if(exact_destination){InventoryNextPickupStatus("Loaded bag needs a whole-container transfer. Source unchanged.");return false;}
        if(slot!=0xffff||custom_slot){InventoryNextPickupStatus("Take the loaded bag into storage before equipping it.");return true;}
        return InventoryNextWorldRequest(source,owner,live);
    }
    const auto source_identity=source==0xffff?u64{0}:InventoryNextIdentity(source),item_identity=InventoryNextIdentity(live);
    if((source!=0xffff&&!source_identity)||!item_identity){InventoryNextPickupStatus("Item unavailable. Transfer cancelled.");return true;}
    NET_Packet packet;CGameObject::u_EventGen(packet,GE_SQA_ACQUIRE,owner);
    packet.w_u16(source);packet.w_u16(live);packet.w_u64(source_identity);packet.w_u64(item_identity);
    packet.w_u16(slot);
    packet.w_u32(custom_slot);packet.w_u64(custom_host);
    packet.w_u64(exact_destination);packet.w_s32(exact_x);packet.w_s32(exact_y);packet.w_u64(expected_logical_owner);
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
    inventory_next::StorageCatalog catalog;if(!read_storage_catalog(catalog))return false;
    if(!catalog.apply(item->m_section_id.c_str(),n)&&smart_cast<CBackpack*>(item)) {
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


bool InventoryNextWorldStorageEnabled() noexcept {
    return InventoryNextLogicalStorage()&&strstr(Core.Params,"-sqa3-world-storage")!=nullptr;
}
bool InventoryNextWorldCanDestroy(u16 live) noexcept try {
    // Destruction during level teardown must remain native. During play, reject
    // the entire recursive deletion before any member or ancestor is removed.
    if(!InventoryNextLogicalStorage()||!g_pGameLevel||!Level().bReady||
       !ai().get_alife())return true;
    if(Actor()&&layout().initialized()){
        auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
        if(item&&item->parent_id()==Actor()->ID()){
            const auto id=InventoryNextIdentityLookup(live);
            if(id)for(const auto& node:layout().model().nodes())if(node.second.parent==id){
                Msg("* SQA3 retained loaded actor container during destruction live=%u",live);return false;
            }
        }
    }
    if(world_storage().records().empty())return true;
    auto protected_ancestor=[live](u64 identity) {
        u16 current=InventoryNextIdentityServer(identity);
        if(current==0xffff)return true;
        for(unsigned depth=0;current!=0xffff;++depth) {
            if(current==live)return true;
            if(depth>=64)return true; // malformed ancestry: fail closed
            auto object=ai().alife().objects().object(current,true);
            if(!object)return true;
            current=object->ID_Parent;
        }
        return false;
    };
    for(const auto& record:world_storage().records()) {
        if(protected_ancestor(record.second.custodian))return false;
        for(const auto& pair:record.second.tree.nodes())
            if(pair.second.kind==inventory_next::Kind::Item&&protected_ancestor(pair.first))return false;
    }
    return true;
} catch (...) {return false;}
bool InventoryNextWorldCanRelease(u16 live) noexcept try {
    if(world_storage().records().empty())return true;
    const auto id=InventoryNextIdentity(live);
    for(const auto& record:world_storage().records())if(record.second.tree.nodes().count(id))return false;
    return true;
} catch (...) {return false;}
bool InventoryNextWorldVisible(u16 live) noexcept try {
    if(world_storage().records().empty())return true;
    const auto id=InventoryNextIdentity(live);
    for(const auto& record:world_storage().records())if(record.second.tree.nodes().count(id)){
        if(record.first==id)return true;
        for(const auto& pair:record.second.tree.nodes())if(pair.second.parent==id)return false;
        return true; // Individual leaf contents may be taken from this stash.
    }
    return true;
} catch (...) {return false;}
bool InventoryNextWorldCanSave() noexcept try {
    if(world_transfer||world_member||ground_provision||pickup_fault)return false;
    for(const auto& record:world_storage().records()) {
        const u16 custodian=InventoryNextIdentityServer(record.second.custodian);
        if(!record.second.custodian||custodian==0xffff||!ai().alife().objects().object(custodian,true))return false;
        for(const auto& pair:record.second.tree.nodes())if(pair.second.kind==inventory_next::Kind::Item) {
            const u16 live=InventoryNextIdentityServer(pair.first);
            auto server=ai().alife().objects().object(live,true);
            if(!server||server->ID_Parent!=custodian)return false;
        }
    }
    return true;
} catch (...) {return false;}
namespace {
void sync_stash_markers() {
    if(!g_pGameLevel||!Level().bReady)return;
    for(auto it=stash_markers.begin();it!=stash_markers.end();) {
        const auto record=world_storage().records().find(it->first);
        if(record==world_storage().records().end()||InventoryNextIdentityServer(record->second.custodian)!=it->second) {
            Level().MapManager().RemoveMapLocation("treasure",it->second);it=stash_markers.erase(it);
        } else ++it;
    }
    for(const auto& pair:world_storage().records()) {
        const auto name=world_storage().stash_name(pair.first);if(name.empty())continue;
        const u16 live=InventoryNextIdentityServer(pair.second.custodian);
        auto server=live==0xffff?nullptr:ai().alife().objects().object(live,true);
        if(!server||xr_strcmp(server->s_name.c_str(),ground_proxy_section))throw std::runtime_error("stash custodian unavailable");
        auto marker=Level().MapManager().GetMapLocation("treasure",live);
        const auto tracked=stash_markers.find(pair.first);
        if(marker&&tracked!=stash_markers.end()&&tracked->second==live)continue;
        if(!marker)marker=Level().MapManager().AddMapLocation("treasure",live);
        if(!marker)throw std::runtime_error("stash marker unavailable");
        marker->SetHint(name.c_str());marker->SetSerializable(false);
        stash_markers[pair.first]=live;
    }
}
bool cleanup_ground_proxy(u16 live) {
    if(live==0xffff)return true;
    auto server=ai().alife().objects().object(live,true);
    if(!server)return true;
    if(xr_strcmp(server->s_name.c_str(),ground_proxy_section)||!server->children.empty())return false;
    const auto identity=InventoryNextIdentity(live);
    for(const auto& record:world_storage().records())if(record.second.custodian==identity)return false;
    auto box=smart_cast<CInventoryBox*>(Level().Objects.net_Find(live));
    if(box) {
        if(!box->IsEmpty())return false;
        if(box->getDestroy())return true;
        if(CurrentGameUI()&&CurrentGameUI()->GetActorMenu().GetInvBox()==box)CurrentGameUI()->HideActorMenu();
        InventoryNextStashForgetMarker(live);box->DestroyObject();
    } else {InventoryNextStashForgetMarker(live);const_cast<CALifeSimulator&>(ai().alife()).release(server,true);}
    Msg("* SQA3 empty ground proxy cleanup live=%u",live);return true;
}
bool world_access(u16 source,u16 destination) {
    if(!Actor()||(source==Actor()->ID())==(destination==Actor()->ID()))return false;
    const u16 box_id=source==Actor()->ID()?destination:source;
    auto object=Level().Objects.net_Find(box_id);
    if(!object||object->getDestroy()||object->Position().distance_to(Actor()->Position())>3.0f)return false;
    if(auto box=smart_cast<CInventoryBox*>(object))return box->can_take()&&!box->closed();
    if(!strstr(Core.Params,"-sqa3-corpse-storage"))return false;
    auto corpse=smart_cast<CEntityAlive*>(object);
    auto owner=smart_cast<CInventoryOwner*>(object);
    return corpse&&!corpse->g_Alive()&&owner&&owner->deadbody_can_take_status();
}
bool world_node(u16 live,u16 parent,inventory_next::Node& node) {
    using namespace inventory_next;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
    auto server=ai().alife().objects().object(live,true);
    if(!item||item->object().getDestroy()||!server||server->ID_Parent!=parent||!item->object().H_Parent()||item->parent_id()!=parent||item->IsQuestItem())return false;
    const auto id=InventoryNextIdentity(live);if(!id)return false;InventoryNextIdentityOnline(*item);
    const double mass=double(item->Weight())*1000.0,condition=item->GetCondition();const auto shape=item->GetInvGridRect();
    if(!std::isfinite(mass)||mass<0||mass>1e12||!std::isfinite(condition)||condition<0||condition>1||shape.x2<1||shape.y2<1||shape.x2>16||shape.y2>64)return false;
    node.id=id;node.parent=1;node.intrinsic=static_cast<Mass>(std::llround(mass));node.condition=static_cast<unsigned>(std::llround(condition*10000));
    node.empty=node.half=node.full={shape.x2,shape.y2};
    node.attributes["native_section"]=item->m_section_id.c_str();node.attributes["native_live_id"]=std::to_string(live);
    node.attributes["native_place"]=parent==Actor()->ID()?std::to_string(item->CurrPlace()):"3";
    node.attributes["native_slot"]=std::to_string(item->CurrSlot());
    native_instance_state(node,item);
    if(parent==Actor()->ID()&&item->CurrPlace()==eItemPlaceSlot) {
        if(item->CurrSlot()>=48)return false;
        node.location=Location::Equipment;node.wear_slots=node.equipped_slots=std::uint64_t{1}<<item->CurrSlot();
    }
    StorageCatalog catalog;if(!read_storage_catalog(catalog))return false;
    if(!catalog.apply(item->m_section_id.c_str(),node)&&smart_cast<CBackpack*>(item)) {
        const std::string section=item->m_section_id.c_str();
        const std::string kind=pSettings->line_exist(section.c_str(),"kind")?pSettings->r_string(section.c_str(),"kind"):"";
        const auto& shape=catalog.resolve(section,kind);node.container=true;node.grid=shape.grid;node.cells=shape.cells;
    }
    apply_custom_binding(node,item);
    return true;
}
void world_refused(const char* message) {
    const u16 proxy=world_transfer&&world_transfer->created_ground_proxy?world_transfer->destination:u16(0xffff);
    world_transfer.reset();
    queue_custom_bindings();
    if(!cleanup_ground_proxy(proxy))pickup_fault=true;
    InventoryNextPickupStatus(message);
}
}
bool InventoryNextValidateDefinition(u16 live) noexcept try {
    if(!InventoryNextLogicalStorage()||!strstr(Core.Params,"-sqa3-fixture")||!Actor())return false;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));if(!item)return false;
    inventory_next::Node node;if(!world_node(live,item->parent_id(),node))return false;
    inventory_next::StorageCatalog catalog;if(!read_storage_catalog(catalog)||!catalog.definitions.count(item->m_section_id.c_str()))return false;
    inventory_next::ActorLayout validation;return validation.reconcile({node},{{4,2},{},true}).ok;
} catch (...) {return false;}
bool InventoryNextWorldRequest(u16 source,u16 destination,u16 live,bool created_ground_proxy,const char* stash_name) noexcept try {
    if(!InventoryNextWorldStorageEnabled()||!Actor())return false;
    if(!InventoryNextPickupCanSave()){InventoryNextPickupStatus("Finish the pending transfer first.");return true;}
    if(!InventoryNextLayoutSync())return true;
    const auto shell=InventoryNextIdentity(live);if(!shell)return false;
    const bool acquire=destination==Actor()->ID();
    if(acquire) {
        if(!world_storage().records().count(shell)) {
            if(!InventoryNextWorldCanRelease(live)){InventoryNextPickupStatus("Take the containing bag first.");return true;}
            return false;
        }
    } else {
        bool loaded=false;for(const auto& pair:layout().model().nodes())if(pair.second.parent==shell){loaded=true;break;}
        if(!loaded&&!(created_ground_proxy&&stash_name&&*stash_name))return false;
    }
    if(!world_access(source,destination)){InventoryNextPickupStatus("Container unavailable. Bag unchanged.");return true;}
    auto prepared=std::make_unique<NativeWorldTransfer>();prepared->source=source;prepared->destination=destination;prepared->acquire=acquire;
    prepared->created_ground_proxy=created_ground_proxy;
    if(stash_name&&*stash_name){if(!created_ground_proxy||acquire||!InventoryNextStashEnabled())return true;prepared->stash_name=stash_name;prepared->stash_shell=live;}
    auto source_object=Level().Objects.net_Find(source);
    prepared->retire_ground_proxy=acquire&&source_object&&!xr_strcmp(source_object->cNameSect().c_str(),ground_proxy_section);
    prepared->custodian=InventoryNextIdentity(acquire?source:destination);
    if(!prepared->custodian)return true;
    std::vector<inventory_next::Id> members;
    if(acquire) {
        const auto& record=world_storage().records().at(shell);
        if(record.custodian!=prepared->custodian){InventoryNextPickupStatus("Bag custody changed. Transfer refused.");return true;}
        for(const auto& pair:record.tree.nodes())if(pair.second.kind==inventory_next::Kind::Item)members.push_back(pair.first);
    } else {
        const auto& nodes=layout().model().nodes();
        for(const auto& pair:nodes) {
            auto id=pair.first;while(id&&id!=shell)id=nodes.at(id).parent;
            if(id==shell)members.push_back(pair.first);
        }
    }
    std::vector<inventory_next::Node> actual;
    for(auto id:members) {
        const u16 member=InventoryNextIdentityServer(id);inventory_next::Node node;
        if(member==0xffff||!world_node(member,source,node)||node.id!=id){InventoryNextPickupStatus("Bag contents unavailable. Transfer refused.");return true;}
        if(acquire) {
            auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(member));
            if(!Actor()->inventory().CanTakeItem(item)){InventoryNextPickupStatus("Bag contents cannot be taken.");return true;}
        }
        actual.push_back(node);prepared->records.push_back({member,id,0,node});
    }
    if(acquire&&!world_storage().refresh(shell,prepared->custodian,actual)){InventoryNextPickupStatus("Bag contents changed. Transfer refused.");return true;}
    auto& registry=prepared->registry;
    if(!registry.add(1)||!registry.bind(1,Actor()->ID())||!registry.add(prepared->custodian)||!registry.bind(prepared->custodian,acquire?source:destination))return true;
    for(const auto& pair:layout().model().nodes())if(pair.second.kind==inventory_next::Kind::Item) {
        const u16 member=InventoryNextIdentityServer(pair.first);
        if(member==0xffff||!registry.add(pair.first)||!registry.bind(pair.first,member))return true;
    }
    for(const auto& record:prepared->records)if(!registry.add(record.identity)||!registry.bind(record.identity,record.live))return true;
    prepared->session=std::make_unique<inventory_next::LoadedStorageSession>(layout().model(),world_storage(),registry);
    std::vector<inventory_next::Id> destinations{1};
    for(const auto& pair:layout().model().nodes())if(pair.second.kind==inventory_next::Kind::Item&&pair.second.container)destinations.push_back(pair.first);
    const auto result=acquire?prepared->session->prepare_acquire(shell,prepared->custodian,destinations):prepared->session->prepare_release(shell,prepared->custodian,prepared->stash_name);
    if(!result){Msg("! SQA3 loaded transfer refused reason=%s",result.error.c_str());InventoryNextPickupStatus("Loaded bag does not fit or is unavailable. Source unchanged.");return true;}
    if(acquire&&!world_storage().stash_name(shell).empty()) {
        luabind::functor<bool> can_remove;
        if(!ai().script_engine().functor("sqa3_stash.can_remove",can_remove)||!can_remove(source)) {
            InventoryNextPickupStatus("Stash removal cancelled. Bag unchanged.");return true;
        }
        if(!InventoryNextPickupCanSave()||!InventoryNextLayoutSync())return true;
    }
    prepared->token=++next_world_token;world_transfer=std::move(prepared);
    NET_Packet packet;CGameObject::u_EventGen(packet,GE_SQA_WORLD_TRANSFER,Actor()->ID());
    packet.w_u16(source);packet.w_u16(destination);packet.w_u64(world_transfer->token);
    InventoryNextPickupStatus("Moving loaded bag...");CGameObject::u_EventSend(packet);return true;
} catch (...) {if(world_transfer)pickup_fault=true;InventoryNextPickupStatus("Loaded transfer needs reconciliation.");return true;}
bool InventoryNextWorldValidate(u16 source,u16 destination,u64 token,xr_vector<u16>& items) noexcept try {
    if(!InventoryNextWorldStorageEnabled()||!world_transfer||world_transfer->committed||world_transfer->token!=token||world_transfer->source!=source||world_transfer->destination!=destination)return false;
    if(!world_access(source,destination)||InventoryNextIdentity(world_transfer->acquire?source:destination)!=world_transfer->custodian||!InventoryNextLayoutSync()) {
        world_refused("Container changed. Bag stayed at source.");return false;
    }
    for(const auto& record:world_transfer->records) {
        inventory_next::Node current;
        if(!world_node(record.live,source,current)||!same_purchase_node(current,record.node)) {
            world_refused("Bag contents changed. Transfer cancelled.");return false;
        }
        items.push_back(record.live);
    }
    auto result=world_transfer->session->dispatch(true,world_transfer->dispatch);
    if(!result){world_refused("Loaded transfer changed. Bag stayed at source.");return false;}
    return true;
} catch (...) {world_refused("Loaded transfer validation failed.");return false;}
bool InventoryNextWorldFinish(bool accepted) noexcept try {
    if(!world_transfer)return false;
    std::vector<inventory_next::TransferSession::ParentObservation> observed;
    for(const auto& member:world_transfer->dispatch.subtree) {
        auto server=ai().alife().objects().object(member.item.live,true);
        if(InventoryNextIdentity(member.item.live)!=member.item.item){pickup_fault=true;return false;}
        observed.push_back({member.item,server?server->ID_Parent:u16(0xffff)});
    }
    const auto result=world_transfer->session->acknowledge(world_transfer->dispatch.token,observed,accepted);
    if(!result) {
        if(world_transfer->session->can_save()) {
            const auto count=world_transfer->records.size();
            world_refused("Loaded transfer refused. Bag unchanged.");
            Msg("* SQA3 loaded transfer rejection restored count=%u save_ready=%d",static_cast<unsigned>(count),InventoryNextPickupCanSave()&&InventoryNextWorldCanSave());
        }
        else {pickup_fault=true;InventoryNextPickupStatus("Loaded transfer needs reconciliation. Saving blocked.");}
        return false;
    }
    const auto actor=world_transfer->session->committed_actor();const auto world=world_transfer->session->committed_world();
    if(!actor||!world||!layout().restore(actor->serialize())){pickup_fault=true;return false;}
    world_storage()=*world;world_transfer->committed=true;
    if(world_transfer->acquire)for(const auto& record:world_transfer->records)pending_pickups.emplace(record.live,actor->nodes().at(record.identity));
    Msg("* SQA3 loaded transfer server committed count=%u source=%u destination=%u; client acknowledgement pending",static_cast<unsigned>(world_transfer->records.size()),world_transfer->source,world_transfer->destination);
    return true;
} catch (...) {pickup_fault=true;return false;}
void InventoryNextWorldClientReady() noexcept try {
    if(!world_transfer||!world_transfer->committed||pickup_fault)return;
    if(world_transfer->acquire&&!pending_pickups.empty())return;
    for(const auto& record:world_transfer->records) {
        auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(record.live));
        if(!item||InventoryNextIdentity(record.live)!=record.identity){pickup_fault=true;return;}
        if(!item->object().H_Parent()||item->parent_id()!=world_transfer->destination)return;
    }
    const auto count=world_transfer->records.size();
    const u16 proxy=world_transfer->retire_ground_proxy?world_transfer->source:u16(0xffff);
    if(!world_transfer->stash_name.empty()) {
        sync_stash_markers();
        auto shell=Level().Objects.net_Find(world_transfer->stash_shell);
        luabind::functor<void> created;
        if(!shell||!ai().script_engine().functor("sqa3_stash.on_created",created))throw std::runtime_error("stash callback unavailable");
        created(world_transfer->destination,world_transfer->stash_name.c_str(),shell->cNameSect().c_str());
    }
    world_transfer.reset();
    if(!cleanup_ground_proxy(proxy)){pickup_fault=true;return;}
    queue_custom_bindings();
    if(!InventoryNextLayoutSync()||!InventoryNextWorldCanSave()){pickup_fault=true;return;}
    InventoryNextPickupStatus("");
    Msg("* SQA3 loaded transfer client completed count=%u save_ready=%d",static_cast<unsigned>(count),InventoryNextPickupCanSave());
} catch (...) {pickup_fault=true;}

bool InventoryNextWorldMemberMove(u16 live,u16 destination,u64 target_shell,int drop_x,int drop_y,u64 view,u64 expected_source_owner) noexcept try {
    using namespace inventory_next;
    if(!InventoryNextWorldStorageEnabled()||!InventoryNextFieldKitEnabled()||!Actor()||
       !InventoryNextPickupCanSave()||!InventoryNextLayoutSync())return false;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
    auto server=ai().alife().objects().object(live,true);
    if(!item||!server||item->object().getDestroy()||item->IsQuestItem())return false;
    const u16 source=item->parent_id();const Id id=InventoryNextIdentity(live);
    if(source==0xffff||!id||server->ID_Parent!=source||!item->object().H_Parent()||
       item->object().H_Parent()->ID()!=source||!smart_cast<CInventoryBox*>(Level().Objects.net_Find(destination))||
       !world_access(Actor()->ID(),destination)||(source!=Actor()->ID()&&!world_access(source,Actor()->ID())))return false;
    const Id source_identity=InventoryNextIdentity(source),destination_identity=InventoryNextIdentity(destination);
    if(!source_identity||!destination_identity)return false;
    const bool actor_source=source==Actor()->ID();
    Id source_shell=0;
    for(const auto& record:world_storage().records())if(record.second.tree.nodes().count(id)){
        if(record.first==id||record.second.custodian!=source_identity||record.second.tree.nodes().at(id).parent!=record.first)
            return false;
        for(const auto& pair:record.second.tree.nodes())if(pair.second.parent==id)return false;
        source_shell=record.first;break;
    }
    if(source_shell!=expected_source_owner||(!actor_source&&!target_shell&&!source_shell))return false;
    if(actor_source&&(!InventoryNextCanRelease(live)||!layout().model().nodes().count(id)))return false;
    if(!actor_source&&!source_shell&&!InventoryNextWorldVisible(live))return false;
    Position position{};
    if(target_shell){
        const auto target=world_storage().records().find(target_shell);
        if(target==world_storage().records().end()||target->second.custodian!=destination_identity||
           drop_x<0||drop_y<0)return false;
        Id owner=0;const auto& selected=make_storage_view(view?view:target_shell);
        if(!selected.destination(drop_x,drop_y,owner,position)||owner!=target_shell)return false;
    }
    Node observed;if(!world_node(live,source,observed)||observed.id!=id)return false;
    if(source_shell){
        const auto& saved=world_storage().records().at(source_shell).tree.nodes().at(id);
        if(!same_purchase_node(saved,observed))return false;
    }
    auto prepared=std::make_unique<NativeWorldMember>();
    prepared->source=source;prepared->destination=destination;prepared->live=live;
    prepared->source_identity=source_identity;prepared->destination_identity=destination_identity;
    prepared->item_identity=id;prepared->source_shell=source_shell;prepared->target_shell=target_shell;
    prepared->actor_source=actor_source;prepared->observed=observed;
    prepared->actor_before=layout().model().serialize();prepared->world_before=world_storage().serialize();
    prepared->actor_candidate=layout().model();
    if(actor_source){Command erase;erase.operation=Operation::Erase;erase.item=id;
        erase.revision=prepared->actor_candidate.revision();erase.session=prepared->actor_candidate.session();
        if(!prepared->actor_candidate.execute(erase))return false;}
    WorldSubtrees candidate=world_storage();
    if(source_shell){WorldSubtrees removed;
        if(!candidate.without_leaf(source_shell,source_identity,id,removed))return false;
        candidate=std::move(removed);}
    if(target_shell){WorldSubtrees inserted;
        auto stored=observed;stored.parent=target_shell;stored.location=Location::Grid;
        stored.attributes["native_place"]="3";
        stored.attributes["native_slot"]="0";
        stored.attributes.erase("native_custom_slot");stored.attributes.erase("native_custom_host");
        stored.attributes.erase("native_custom_key");
        if(!candidate.with_leaf(target_shell,destination_identity,stored,position,inserted)){
            InventoryNextPickupStatus("That item does not fit in the box. Source unchanged.");return false;}
        candidate=std::move(inserted);}
    prepared->world_candidate=std::move(candidate);
    if(source==destination){
        if(actor_source||layout().model().serialize()!=prepared->actor_before||
           world_storage().serialize()!=prepared->world_before)return false;
        world_storage()=std::move(prepared->world_candidate);storage_views.clear();
        InventoryNextPickupStatus("");Msg("* SQA3 detached member repositioned live=%u source=%llu destination=%llu",live,source_shell,target_shell);
        return true;
    }
    prepared->token=++next_world_member_token;world_member=std::move(prepared);
    NET_Packet packet;CGameObject::u_EventGen(packet,GE_SQA_WORLD_MEMBER,Actor()->ID());
    packet.w_u16(source);packet.w_u16(destination);packet.w_u16(live);packet.w_u64(world_member->token);
    InventoryNextPickupStatus("Moving item...");CGameObject::u_EventSend(packet);return true;
} catch (...) {if(world_member)pickup_fault=true;InventoryNextPickupStatus("Direct box transfer needs reconciliation.");return false;}
bool InventoryNextWorldMemberValidate(u16 source,u16 destination,u16 live,u64 token) noexcept try {
    if(!world_member||world_member->committed||world_member->token!=token||
       world_member->source!=source||world_member->destination!=destination||world_member->live!=live)return false;
    if(!Actor()||!world_access(Actor()->ID(),destination)||
       (source!=Actor()->ID()&&!world_access(source,Actor()->ID()))||
       InventoryNextIdentity(source)!=world_member->source_identity||
       InventoryNextIdentity(destination)!=world_member->destination_identity||
       InventoryNextIdentity(live)!=world_member->item_identity||
       layout().model().serialize()!=world_member->actor_before||
       world_storage().serialize()!=world_member->world_before)return false;
    inventory_next::Node current;
    return world_node(live,source,current)&&same_purchase_node(current,world_member->observed);
} catch (...) {return false;}
bool InventoryNextWorldMemberFinish(bool accepted) noexcept try {
    if(!world_member)return false;
    auto server=ai().alife().objects().object(world_member->live,true);
    if(!server||server->ID_Parent!=(accepted?world_member->destination:world_member->source)){
        pickup_fault=true;InventoryNextPickupStatus("Direct box transfer needs reconciliation.");return false;}
    if(!accepted){world_member.reset();InventoryNextPickupStatus("Item stayed at source.");return true;}
    if(layout().model().serialize()!=world_member->actor_before||
       world_storage().serialize()!=world_member->world_before||
       (world_member->actor_source&&!layout().restore(world_member->actor_candidate.serialize()))){
        pickup_fault=true;return false;}
    world_storage()=std::move(world_member->world_candidate);storage_views.clear();
    world_member->committed=true;
    Msg("* SQA3 direct world member accepted live=%u source=%u destination=%u",world_member->live,world_member->source,world_member->destination);
    return true;
} catch (...) {pickup_fault=true;return false;}
void InventoryNextWorldMemberClientReady() noexcept try {
    if(!world_member||!world_member->committed||pickup_fault)return;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(world_member->live));
    if(!item||InventoryNextIdentity(world_member->live)!=world_member->item_identity){pickup_fault=true;return;}
    if(item->parent_id()!=world_member->destination)return;
    const u16 live=world_member->live;world_member.reset();
    if(!InventoryNextLayoutSync()||!InventoryNextWorldCanSave()){pickup_fault=true;return;}
    InventoryNextPickupStatus("");Msg("* SQA3 direct world member client ready live=%u save_ready=%d",live,InventoryNextPickupCanSave());
} catch (...) {pickup_fault=true;}

bool InventoryNextGroundStorageEnabled() noexcept {
    return InventoryNextWorldStorageEnabled()&&strstr(Core.Params,"-sqa3-ground-storage")!=nullptr;
}
static bool begin_ground_drop(u16 live,const char* stash_name) noexcept try {
    if(!InventoryNextGroundStorageEnabled()||!Actor())return false;
    if(!InventoryNextPickupCanSave()){InventoryNextPickupStatus("Finish the pending transfer first.");return true;}
    if(!InventoryNextLayoutSync())return true;
    const auto shell=InventoryNextIdentity(live);bool loaded=false;
    for(const auto& pair:layout().model().nodes())if(pair.second.parent==shell){loaded=true;break;}
    if(!loaded&&!stash_name)return false;
    const auto found=layout().model().nodes().find(shell);
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
    if(found==layout().model().nodes().end()||!found->second.container||!item||item->IsQuestItem()||item->parent_id()!=Actor()->ID())return true;
    if(!pSettings->section_exist(ground_proxy_section)){InventoryNextPickupStatus("Ground bag configuration unavailable. Bag unchanged.");return true;}
    auto pending=std::make_unique<NativeGroundProvision>();pending->shell=live;pending->shell_identity=shell;
    if(stash_name)pending->stash_name=stash_name;
    pending->started=Device.dwTimeGlobal;pending->actor_before=layout().model().serialize();ground_provision=std::move(pending);
    const auto& location=Actor()->ai_location();
    auto server=const_cast<CALifeSimulator&>(ai().alife()).spawn_item(ground_proxy_section,Actor()->Position(),location.level_vertex_id(),location.game_vertex_id(),0xffff);
    if(!server){ground_provision.reset();InventoryNextPickupStatus("Cannot place the bag here. Bag unchanged.");return true;}
    ground_provision->proxy=server->ID;
    InventoryNextIdentity(server->ID);
    InventoryNextPickupStatus("Placing loaded bag...");
    Msg("* SQA3 ground bag provisioning shell=%u proxy=%u; save blocked",live,server->ID);return true;
} catch (...) {pickup_fault=true;InventoryNextPickupStatus("Ground bag placement needs reconciliation.");return true;}
bool InventoryNextGroundDrop(u16 live) noexcept {return begin_ground_drop(live,nullptr);}
bool InventoryNextStashEnabled() noexcept {
    return InventoryNextGroundStorageEnabled()&&strstr(Core.Params,"-sqa3-stash-storage")!=nullptr;
}
std::string InventoryNextStashIdentity(u16 live) noexcept try {
    if(!InventoryNextStashEnabled())return {};
    const auto id=InventoryNextIdentity(live);return id?std::to_string(id):std::string{};
} catch (...) {return {};}
bool InventoryNextStashCreate(u16 live,const char* expected_identity,const char* name) noexcept try {
    if(!InventoryNextStashEnabled())return false;
    const auto identity=InventoryNextStashIdentity(live);
    if(identity.empty()||!expected_identity||identity!=expected_identity){InventoryNextPickupStatus("Backpack changed. Stash creation cancelled.");return false;}
    if(!name||!*name||xr_strlen(name)>240){InventoryNextPickupStatus("Stash name is empty or too long.");return false;}
    for(const unsigned char* p=reinterpret_cast<const unsigned char*>(name);*p;++p)
        if(*p<32||*p==127){InventoryNextPickupStatus("Stash name contains an unsupported character.");return false;}
    return begin_ground_drop(live,name);
} catch (...) {pickup_fault=true;return false;}
void InventoryNextStashForgetMarker(u16 live) noexcept try {
    for(auto it=stash_markers.begin();it!=stash_markers.end();) {
        if(it->second==live) {
            if(g_pGameLevel&&Level().bReady)Level().MapManager().RemoveMapLocation("treasure",live);
            it=stash_markers.erase(it);
        } else ++it;
    }
} catch (...) {pickup_fault=true;}
void InventoryNextGroundUpdate() noexcept try {
    InventoryNextWorldMemberClientReady();
    if(InventoryNextStashEnabled()&&!world_transfer&&!ground_provision&&Actor()&&g_pGameLevel&&Level().bReady)sync_stash_markers();
    if(!ground_provision||!Actor()||!g_pGameLevel)return;
    const u16 proxy=ground_provision->proxy,shell=ground_provision->shell;
    const bool timed_out=Device.dwTimeGlobal-ground_provision->started>5000;
    const bool current=InventoryNextIdentity(shell)==ground_provision->shell_identity&&InventoryNextLayoutSync()&&
        layout().model().serialize()==ground_provision->actor_before;
    if(!current||timed_out) {
        ground_provision.reset();if(!cleanup_ground_proxy(proxy))pickup_fault=true;
        InventoryNextPickupStatus("Ground placement changed or timed out. Bag stayed carried.");return;
    }
    if(strstr(Core.Params,"-sqa3-fixture-ground-proxy-timeout"))return;
    auto box=smart_cast<CInventoryBox*>(Level().Objects.net_Find(proxy));
    if(!box)return;
    const auto identity=ground_provision->shell_identity;const auto name=ground_provision->stash_name;
    ground_provision.reset();
    InventoryNextWorldRequest(Actor()->ID(),proxy,shell,true,name.empty()?nullptr:name.c_str());
    if(!world_transfer&&!world_storage().records().count(identity)&&!cleanup_ground_proxy(proxy))pickup_fault=true;
} catch (...) {pickup_fault=true;InventoryNextPickupStatus("Ground bag placement needs reconciliation.");}

bool InventoryNextFieldKitEnabled() noexcept {
    return InventoryNextLogicalStorage()&&strstr(Core.Params,"-sqa3-field-kit")!=nullptr;
}
u64 InventoryNextContainer(u16 live) noexcept try {
    if(!InventoryNextFieldKitEnabled()||!layout_current)return 0;
    const auto id=InventoryNextIdentity(live);const auto found=layout().model().nodes().find(id);
    if(found==layout().model().nodes().end()&&world_storage().records().count(id))return id;
    if(found==layout().model().nodes().end()||!found->second.container)return 0;
    const auto family=found->second.attributes.find("storage_family");
    return found->second.location!=inventory_next::Location::Equipment||(family!=found->second.attributes.end()&&family->second=="rig")?id:0;
} catch (...) {return 0;}
u64 InventoryNextViewOwner(u16 live) noexcept try {
    if(!InventoryNextFieldKitEnabled()||!layout_current)return 0;
    const auto found=layout().model().nodes().find(InventoryNextIdentity(live));
    if(found==layout().model().nodes().end()||found->second.location!=inventory_next::Location::Grid)return 0;
    inventory_next::Position p;if(storage_view.position(found->second,p))return 0;
    return found->second.parent;
} catch (...) {return 0;}
bool InventoryNextWorldView(u64 owner) noexcept try {
    return InventoryNextFieldKitEnabled()&&world_storage().records().count(owner)!=0;
} catch (...) {return false;}
xr_vector<u16> InventoryNextWorldContents(u64 owner) noexcept try {
    xr_vector<u16> result;const auto record=world_storage().records().find(owner);
    if(record==world_storage().records().end())return result;
    for(const auto& pair:record->second.tree.nodes())if(pair.second.parent==owner){
        const u16 live=InventoryNextIdentityServer(pair.first);
        if(live!=0xffff)result.push_back(live);
    }
    return result;
} catch (...) {return {};}
u16 InventoryNextWorldCustodianLive(u64 owner) noexcept try {
    const auto record=world_storage().records().find(owner);
    return record==world_storage().records().end()?u16(0xffff):InventoryNextIdentityServer(record->second.custodian);
} catch (...) {return 0xffff;}
void InventoryNextStorageToggle(u64 owner) noexcept try {
    if(!InventoryNextFieldKitEnabled()||!owner||CUIDragDropListEx::m_drag_item)return;
    storage_views.clear();
    if(collapsed_storage.count(owner))collapsed_storage.erase(owner);else collapsed_storage.insert(owner);
    Msg("* SQA3 storage owner=%llu collapsed=%d; presentation only",owner,int(collapsed_storage.count(owner)!=0));
    if(CurrentGameUI())CurrentGameUI()->GetActorMenu().m_sqaRefresh=true;
} catch (...) {}

void move_item_from_to(u16,u16,u16);
xr_vector<InventoryNextLootSource> InventoryNextNearbyLoot(u16 corpse) noexcept try {
    xr_vector<InventoryNextLootSource> result;
    if(!InventoryNextFieldKitEnabled()||!InventoryNextStrictOwnedPickup()||!Actor())return result;
    source_footprints.clear();
    inventory_next::StorageCatalog catalog;if(!read_storage_catalog(catalog))return result;
    auto observe=[&](CInventoryItem* item){
        if(!item)return;
        const auto id=InventoryNextIdentity(item->object_id());if(!id)return;
        for(const auto& record:world_storage().records())if(record.second.tree.nodes().count(id)){
            source_footprints[id]=record.second.tree.footprint(id);return;
        }
        const auto definition=catalog.definitions.find(item->m_section_id.c_str());
        if(definition!=catalog.definitions.end())source_footprints[id]=definition->second.empty;
    };
    InventoryNextLootSource ground;ground.name="Ground";
    for(u32 n=0;n<Level().Objects.o_count();++n){
        auto object=Level().Objects.o_get_by_iterator(n);if(!object||object->getDestroy())continue;
        if(Actor()->SqaCanPickup(object->ID())){auto item=smart_cast<CInventoryItem*>(object);observe(item);ground.items.push_back(object->ID());ground.grams+=static_cast<s64>(std::llround(item->Weight()*1000.0));}
        auto box=smart_cast<CInventoryBox*>(object);
        if(!box&&object->ID()!=corpse)continue;
        if(!world_access(object->ID(),Actor()->ID()))continue;
        InventoryNextLootSource source;source.live=object->ID();source.identity=InventoryNextIdentity(source.live);
        if(!source.identity)continue;
        source.name=box?"Stash":"Corpse";
        if(!box)if(auto owner=smart_cast<CInventoryOwner*>(object))if(owner->Name()&&*owner->Name())source.name=owner->Name();
        if(box)for(const auto& record:world_storage().records())if(record.second.custodian==source.identity){
            const auto name=world_storage().stash_name(record.first);if(!name.empty()){source.name=name;break;}
        }
        TIItemContainer items;
        if(box)box->AddAvailableItems(items);else if(auto owner=smart_cast<CInventoryOwner*>(object))owner->inventory().AddAvailableItems(items,false,false);
        for(auto item:items)if(item){
            source.grams+=static_cast<s64>(std::llround(item->Weight()*1000.0));
            const auto identity=InventoryNextIdentity(item->object_id());bool nested=false;
            for(const auto& record:world_storage().records())if(record.first!=identity&&record.second.tree.nodes().count(identity)){nested=true;break;}
            if(!nested&&InventoryNextWorldVisible(item->object_id())){observe(item);source.items.push_back(item->object_id());}
        }
        std::sort(source.items.begin(),source.items.end());result.push_back(source);
        if(box)for(const auto& record:world_storage().records())if(record.second.custodian==source.identity){
            const u16 shell_live=InventoryNextIdentityServer(record.first);
            auto shell=smart_cast<CInventoryItem*>(Level().Objects.net_Find(shell_live));
            if(!shell||shell->parent_id()!=source.live)continue;
            InventoryNextLootSource interior;interior.live=source.live;interior.identity=source.identity;
            interior.logical_owner=record.first;interior.name=std::string("Inside ")+(shell->NameItem()?shell->NameItem():"box");
            const auto& nodes=record.second.tree.nodes();
            for(const auto& entry:nodes)if(entry.second.parent==record.first){
                const u16 member=InventoryNextIdentityServer(entry.first);
                auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(member));
                if(item&&item->parent_id()==source.live&&InventoryNextWorldVisible(member)){
                    interior.items.push_back(member);interior.grams+=record.second.tree.subtree_mass(entry.first);
                    observe(item);
                }
            }
            std::sort(interior.items.begin(),interior.items.end());result.push_back(std::move(interior));
        }
    }
    std::sort(result.begin(),result.end(),[](const InventoryNextLootSource& a,const InventoryNextLootSource& b){return a.live<b.live||(a.live==b.live&&a.logical_owner<b.logical_owner);});
    unsigned unnamed=0;for(auto& source:result)if(source.name=="Stash")source.name+=" "+std::to_string(++unnamed);
    if(!ground.items.empty()){std::sort(ground.items.begin(),ground.items.end());result.insert(result.begin(),std::move(ground));}
    return result;
} catch (...) {return {};}
bool InventoryNextLootTake(u16 source,u64 source_identity,u16 live,u64 item_identity,u16 slot,u32 custom_slot,u64 custom_host,int drop_x,int drop_y,u64 view,u64 expected_logical_owner) noexcept try {
    if(!InventoryNextFieldKitEnabled()||!Actor()||!InventoryNextPickupCanSave()||!item_identity||InventoryNextIdentity(live)!=item_identity)return false;
    if(source==0xffff){if(custom_slot)return InventoryNextRequestOwnedPickup(source,Actor()->ID(),live,slot,custom_slot,custom_host);if(slot!=0xffff){InventoryNextPickupStatus("Take this ground item into storage before equipping it.");return false;}return Actor()->SqaPickup(live);}
    if(!source_identity||InventoryNextIdentity(source)!=source_identity||!world_access(source,Actor()->ID()))return false;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
    if(!item||item->parent_id()!=source||!InventoryNextWorldVisible(live))return false;
    inventory_next::Id destination=0;inventory_next::Position local;
    if(drop_x>=0||drop_y>=0){
        if(drop_x<0||drop_y<0||slot!=0xffff||custom_slot||!InventoryNextLayoutSync())return false;
        const auto& selected=view?make_storage_view(view):storage_view;
        if(!selected.destination(drop_x,drop_y,destination,local)){InventoryNextPickupStatus("Drop on an open storage cell. Source unchanged.");return false;}
    }
    return InventoryNextRequestOwnedPickup(source,Actor()->ID(),live,slot,custom_slot,custom_host,destination,local.x,local.y,expected_logical_owner);
} catch (...) {return false;}
bool InventoryNextLootCustomEquip(u16 source,u64 source_identity,u16 live,u64 item_identity,u64 host,const char* key,u64 expected_logical_owner) noexcept try {
    if(!key||!read_equipment_registry()||!Actor())return false;
    const auto definition=equipment_registry.find(std::string(key));if(!definition||!definition->enabled)return false;
    const auto slot=definition->id;
    if(source==0xffff) {
        if(!Actor()->SqaCanPickup(live)||InventoryNextIdentityLookup(live)!=item_identity)return false;
        auto object=smart_cast<CGameObject*>(Level().Objects.net_Find(live));luabind::functor<bool> callback;
        if(ai().script_engine().functor("bind_stalker_ext.actor_on_item_before_pickup",callback)&&!callback(object->lua_game_object()))return false;
        if(!Actor()->SqaCanPickup(live)||InventoryNextIdentityLookup(live)!=item_identity)return false;
        if(auto usable=smart_cast<CUsableScriptObject*>(object))usable->use(Actor());
        if(!Actor()->SqaCanPickup(live)||InventoryNextIdentityLookup(live)!=item_identity)return false;
    }
    return InventoryNextLootTake(source,source_identity,live,item_identity,0xffff,slot,host,-1,-1,0,expected_logical_owner);
} catch (...) {return false;}
bool InventoryNextLootPut(u16 source,u64 source_identity,u16 live,int drop_x,int drop_y,u64 target_shell,u64 source_owner) noexcept try {
    if(!InventoryNextFieldKitEnabled()||!Actor()||!InventoryNextPickupCanSave())return false;
    auto item=smart_cast<CInventoryItem*>(Level().Objects.net_Find(live));
    if(!item||item->IsQuestItem())return false;
    if(source==0xffff){InventoryNextPickupStatus("Use the Drop item area to put items on the ground.");return false;}
    if(!source_identity||InventoryNextIdentity(source)!=source_identity||!world_access(Actor()->ID(),source))return false;
    if(target_shell||source_owner)return InventoryNextWorldMemberMove(live,source,target_shell,drop_x,drop_y,target_shell,source_owner);
    if(item->parent_id()!=Actor()->ID())return false;
    if(InventoryNextWorldRequest(Actor()->ID(),source,live))return true;
    if(!InventoryNextCanRelease(live))return false;
    move_item_from_to(Actor()->ID(),source,live);return true;
} catch (...) {return false;}

int InventoryNextNativePreview(u16 live,int x,int y,u64 view,int& width,int& height) noexcept try {
    width=height=0;
    // Draw runs alongside parallel Lua GC/map updates. Enrollment calls the
    // script-overridable can_save(), so rendering must only read existing IDs.
    if(!InventoryNextFieldKitEnabled()||!layout_current||native_drag().item()!=InventoryNextIdentityLookup(live))return 0;
    const auto& model=layout().model();const auto id=native_drag().item();
    if(!model.nodes().count(id))return 0;
    const auto shape=model.footprint(id);width=shape.w;height=shape.h;
    const auto& selected=view?make_storage_view(view):storage_view;
    inventory_next::Id owner=0;inventory_next::Position local;
    if(!selected.destination(x,y,owner,local))return -1;
    const auto proposal=native_drag().preview(model,owner,local.x,local.y,false,{});
    return proposal.result.ok?1:-1;
} catch (...) {return -1;}

void InventoryNextStorageMode(bool separate) noexcept {fieldkit_equipped_view=separate;}
