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
// V1 CONSTRAINTS (enforced by the mod's scripts, documented here):
//   * a container belongs to the actor or lies on the ground - it is
//     never given to an NPC (the NPC offline path walks children one
//     level deep and would strand the contents);
//   * a container never holds another container (the switch overrides
//     recurse one level, matching that rule).
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
	// CInventory::CalcTotalWeight is only re-run when the OWNER's
	// inventory changes shape. Putting something into a container does
	// not change the owner's inventory shape, so the total would go
	// stale until the next take or drop - this pokes it awake.
	void RecalcOwnerWeight();

	DECLARE_SCRIPT_REGISTER_FUNCTION
};
