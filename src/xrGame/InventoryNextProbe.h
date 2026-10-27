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
