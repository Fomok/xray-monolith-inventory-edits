#pragma once
// Explicit diagnostic command only; never registered without -inventory-lab -dbg.
void InventoryNextProbe(bool export_snapshot, bool show_view = false) noexcept;

class IReader;class IWriter;
bool InventoryNextLayoutSync() noexcept;
void InventoryNextLayoutReset() noexcept;
void InventoryNextLayoutSave(IWriter&) noexcept;
void InventoryNextLayoutLoad(IReader&) noexcept;
void InventoryNextMove(u64 id,int x,int y,bool rotated) noexcept;

bool InventoryNextNativePosition(u16 live,int& x,int& y,bool& rotated) noexcept;
bool InventoryNextNativeDrop(u16 live,int x,int y) noexcept;
bool InventoryNextNativeDragBegin(u16 live) noexcept;
bool InventoryNextNativeDragCurrent(u16 live) noexcept;
bool InventoryNextNativeRotate(u16 live) noexcept;
void InventoryNextCapacityProbe() noexcept;
bool InventoryNextLogicalStorage() noexcept;
bool InventoryNextCanRelease(u16 live) noexcept;
void InventoryNextStore(u64 id,u64 destination,int x,int y,bool rotated) noexcept;
class CUIDragDropListEx;
void InventoryNextStorageView(CUIDragDropListEx* list) noexcept;
int InventoryNextStorageCell(int x,int y) noexcept;

// Opt-in ground ownership path; trade/loot pre-detach admission is separate.
bool InventoryNextStrictPickup() noexcept;
bool InventoryNextPickupPrepare(u16 owner,u16 live,u16 source=0xffff) noexcept;
void InventoryNextPickupFinish(u16 owner,u16 live,bool accepted) noexcept;
void InventoryNextPickupTaken(u16 live) noexcept;
void InventoryNextPickupForget(u16 live) noexcept;
bool InventoryNextPickupCanSave() noexcept;

bool InventoryNextStrictOwnedPickup() noexcept;
bool InventoryNextRequestOwnedPickup(u16 source,u16 owner,u16 live) noexcept;
void InventoryNextPickupStatus(const char* text) noexcept;
