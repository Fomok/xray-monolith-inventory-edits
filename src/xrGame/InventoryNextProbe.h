#pragma once
#include <string>
#include <vector>
// Explicit diagnostic command only; never registered without -inventory-lab -dbg.
void InventoryNextProbe(bool export_snapshot, bool show_view = false) noexcept;

class IReader;class IWriter;
bool InventoryNextLayoutSync() noexcept;
void InventoryNextLayoutReset() noexcept;
void InventoryNextLayoutSave(IWriter&) noexcept;
void InventoryNextLayoutLoad(IReader&) noexcept;
void InventoryNextMove(u64 id,int x,int y,bool rotated) noexcept;

bool InventoryNextNativePosition(u16 live,int& x,int& y,bool& rotated,u64 view=0) noexcept;
bool InventoryNextNativeFootprint(u16 live,int& width,int& height) noexcept;
bool InventoryNextCanCustomEquip(u64 host,u32 slot,u16 live,bool restoring=false) noexcept;
bool InventoryNextCanCustomSwap(u64 host,u32 slot,u16 incoming) noexcept;
void InventoryNextCustomChanged() noexcept;
void InventoryNextConditionChanged(u16 live) noexcept;
void InventoryNextEquipmentUpdate() noexcept;
void InventoryNextRigHit(float power,int hit_type) noexcept;
bool InventoryNextActionReady() noexcept;
bool InventoryNextAmmoAccessible(u16 live) noexcept;
bool InventoryNextCanUse(u16 live) noexcept;
u16 InventoryNextQuickPick(const char* section) noexcept;
bool InventoryNextCustomEquip(u16 live,u64 host,const char* key) noexcept;
bool InventoryNextCustomUnequip(u16 live,int x=-1,int y=-1,u64 view=0) noexcept;
bool InventoryNextManualUnequip(u16 live,int x=-1,int y=-1,u64 view=0) noexcept;
u16 InventoryNextCustomItem(u64 host,const char* key) noexcept;
struct InventoryNextEquipmentAction {std::string key,label;u64 host=1,identity=0;u16 live=0xffff;bool detach=false,open=false,acquire=false;};
std::vector<InventoryNextEquipmentAction> InventoryNextEquipmentActions(u16 live) noexcept;
bool InventoryNextNativeDrop(u16 live,int x,int y,u64 view=0) noexcept;
bool InventoryNextNativeDragBegin(u16 live) noexcept;
bool InventoryNextNativeDragCurrent(u16 live) noexcept;
bool InventoryNextNativeRotate(u16 live) noexcept;
void InventoryNextCapacityProbe() noexcept;
bool InventoryNextLogicalStorage() noexcept;
bool InventoryNextCanRelease(u16 live) noexcept;
bool InventoryNextCanBulkRelease(u16 live) noexcept;
void InventoryNextStore(u64 id,u64 destination,int x,int y,bool rotated) noexcept;
class CUIDragDropListEx;
void InventoryNextStorageView(CUIDragDropListEx* list,u64 view=0) noexcept;
bool InventoryNextStorageExtent(u64 view,int& width,int& height) noexcept;
int InventoryNextStorageCell(int x,int y,u64 view=0) noexcept;

// Opt-in ground ownership path; trade/loot pre-detach admission is separate.
bool InventoryNextStrictPickup() noexcept;
bool InventoryNextPickupPrepare(u16 owner,u16 live,u16 source=0xffff,u16 slot=0xffff,u32 custom_slot=0,u64 custom_host=1) noexcept;
u16 InventoryNextPickupSlot(u16 owner,u16 live) noexcept;
bool InventoryNextPickupCustomPending(u16 owner,u16 live) noexcept;
void InventoryNextPickupFinish(u16 owner,u16 live,bool accepted) noexcept;
void InventoryNextPickupTaken(u16 live) noexcept;
void InventoryNextPickupForget(u16 live) noexcept;
bool InventoryNextPickupCanSave() noexcept;

bool InventoryNextStrictOwnedPickup() noexcept;
bool InventoryNextRequestOwnedPickup(u16 source,u16 owner,u16 live,u16 slot=0xffff,u32 custom_slot=0,u64 custom_host=1) noexcept;
void InventoryNextPickupStatus(const char* text) noexcept;

class CTrade;
bool InventoryNextStrictTradePickup() noexcept;
bool InventoryNextPurchaseRequest(CTrade* trade,const xr_vector<u16>& items) noexcept;
bool InventoryNextPurchaseValidate(u16 source,u16 owner,u64 token,xr_vector<u16>& items) noexcept;
bool InventoryNextPurchaseFinish(bool accepted) noexcept;
void InventoryNextPurchaseClientReady() noexcept;
void InventoryNextPurchaseProbe(u16 source,const xr_vector<u16>& items) noexcept;
bool InventoryNextWorldStorageEnabled() noexcept;
bool InventoryNextWorldRequest(u16 source,u16 destination,u16 live,bool created_ground_proxy=false,const char* stash_name=nullptr) noexcept;
bool InventoryNextWorldValidate(u16 source,u16 destination,u64 token,xr_vector<u16>& items) noexcept;
bool InventoryNextWorldFinish(bool accepted) noexcept;
void InventoryNextWorldClientReady() noexcept;
bool InventoryNextWorldCanSave() noexcept;
bool InventoryNextWorldCanDestroy(u16 live) noexcept;
bool InventoryNextWorldCanRelease(u16 live) noexcept;
bool InventoryNextWorldVisible(u16 live) noexcept;
bool InventoryNextGroundStorageEnabled() noexcept;
bool InventoryNextGroundDrop(u16 live) noexcept;
void InventoryNextGroundUpdate() noexcept;
bool InventoryNextStashEnabled() noexcept;
std::string InventoryNextStashIdentity(u16 live) noexcept;
bool InventoryNextStashCreate(u16 live,const char* expected_identity,const char* name) noexcept;
void InventoryNextStashForgetMarker(u16 live) noexcept;

bool InventoryNextFieldKitEnabled() noexcept;
u64 InventoryNextContainer(u16 live) noexcept;
u64 InventoryNextViewOwner(u16 live) noexcept;
void InventoryNextStorageToggle(u64 owner) noexcept;

struct InventoryNextLootSource {u16 live=0xffff;u64 identity=0;s64 grams=0;std::string name;xr_vector<u16> items;};
xr_vector<InventoryNextLootSource> InventoryNextNearbyLoot(u16 corpse=0xffff) noexcept;
bool InventoryNextLootTake(u16 source,u64 source_identity,u16 live,u64 item_identity,u16 slot=0xffff,u32 custom_slot=0,u64 custom_host=1) noexcept;
bool InventoryNextLootCustomEquip(u16 source,u64 source_identity,u16 live,u64 item_identity,u64 host,const char* key) noexcept;
bool InventoryNextLootPut(u16 source,u64 source_identity,u16 live) noexcept;

int InventoryNextNativePreview(u16 live,int x,int y,u64 view,int& width,int& height) noexcept;

void InventoryNextStorageMode(bool separate) noexcept;
