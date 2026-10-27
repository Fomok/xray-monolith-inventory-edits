#pragma once
class IReader;
class IWriter;
// Developer mode only; saves with this chunk require the same explicit mode.
bool InventoryNextEnabled() noexcept;
u64 InventoryNextIdentity(u16 server_id) noexcept;
void InventoryNextIdentityReset() noexcept;
void InventoryNextIdentityRetire(u16 server_id) noexcept;
void InventoryNextIdentityRemap(u16 before, u16 after) noexcept;
bool InventoryNextIdentityCanSave() noexcept;
void InventoryNextIdentitySave(IWriter& stream) noexcept;
void InventoryNextIdentityLoad(IReader& stream) noexcept;
void InventoryNextIdentityValidate() noexcept;

class CInventoryItem;
namespace inventory_next { class EntityRegistry; }
void InventoryNextIdentityOnline(CInventoryItem& item) noexcept;
void InventoryNextIdentityOffline(CInventoryItem& item) noexcept;
const inventory_next::EntityRegistry& InventoryNextLiveRegistry() noexcept;
