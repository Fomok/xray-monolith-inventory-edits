////////////////////////////////////////////////////////////////////////////
// InventoryContainer.h: a stash box you can carry.
//
// AMP: an inventory item that OWNS other inventory items, the way
// CInventoryBox (the stash) owns its contents: children are real
// objects, H_SetParent'ed to this item, hidden and inert, travelling
// with it through saves and level changes on the engine's own
// ownership machinery.
//
// The client-side contract is exactly the box's: keep the id list,
// H_SetParent on TAKE/BUY, H_SetParent(NULL) on REJECT/SELL. What is
// new against the box is only what "carryable" adds: this class IS an
// inventory item, and its Weight() answers for everything inside it.
//
// Portable containers may own other portable containers. The standard
// ownership and ALife switch paths preserve their complete child trees;
// SqaStorageTransfer prevents cycles before sending ownership events.
// NPC ownership remains unsupported by the public mod's transfer UI.
////////////////////////////////////////////////////////////////////////////
#pragma once
#include "inventory_item_object.h"
#include "script_export_space.h"

class CInventoryContainer : public CInventoryItemObject
{
	typedef CInventoryItemObject inherited;

public:
	xr_vector<u16> m_items;

	CInventoryContainer();
	virtual ~CInventoryContainer();

	virtual void OnEvent(NET_Packet& P, u16 type);
	virtual BOOL net_Spawn(CSE_Abstract* DC);
	virtual void net_Destroy();

	// The whole point of carrying things: they weigh what they weigh.
	// CInventory::CalcTotalWeight calls this through the base pointer,
	// so the contents count against the actor like anything else.
	virtual float Weight() const;

	IC bool IsEmpty() const { return m_items.empty(); }

	virtual CInventoryContainer* cast_inventory_container() { return this; }
	virtual CGameObject* cast_game_object() { return this; }

private:
	// An inner container has no inventory pointer; walk through enclosing
	// containers to invalidate the carrying actor's total weight.
	void RecalcOwnerWeight();

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
