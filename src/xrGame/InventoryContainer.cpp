////////////////////////////////////////////////////////////////////////////
// InventoryContainer.cpp: a stash box you can carry. See the header
// for the design; the ownership handling below is CInventoryBox's,
// which is the engine's one proven way to own items.
////////////////////////////////////////////////////////////////////////////
#include "pch_script.h"
#include "InventoryContainer.h"
#include "level.h"
#include "actor.h"
#include "game_object_space.h"
#include "inventory_item.h"
#include "Inventory.h"

CInventoryContainer::CInventoryContainer()
{
}

CInventoryContainer::~CInventoryContainer()
{
}

void CInventoryContainer::OnEvent(NET_Packet& P, u16 type)
{
	inherited::OnEvent(P, type);

	// Events routed TO this object are about its children - our own
	// pickup/trade events go to the actor, not to us - so the box's
	// handling applies unchanged.
	switch (type)
	{
	case GE_TRADE_BUY:
	case GE_OWNERSHIP_TAKE:
		{
			u16 id;
			P.r_u16(id);
			CObject* itm = Level().Objects.net_Find(id);
			VERIFY(itm);
			if (!itm) break;
			m_items.push_back(id);
			itm->H_SetParent(this);
			itm->setVisible(FALSE);
			itm->setEnabled(FALSE);
			RecalcOwnerWeight();
		}
		break;

	case GE_TRADE_SELL:
	case GE_OWNERSHIP_REJECT:
		{
			u16 id;
			P.r_u16(id);
			CObject* itm = Level().Objects.net_Find(id);
			VERIFY(itm);
			if (!itm) break;
			xr_vector<u16>::iterator it = std::find(m_items.begin(), m_items.end(), id);
			VERIFY(it != m_items.end());
			if (it != m_items.end())
				m_items.erase(it);

			bool just_before_destroy = !P.r_eof() && P.r_u8();
			bool dont_create_shell = (type == GE_TRADE_SELL) || just_before_destroy;

			itm->H_SetParent(NULL, dont_create_shell);
			RecalcOwnerWeight();
		}
		break;
	};
}

BOOL CInventoryContainer::net_Spawn(CSE_Abstract* DC)
{
	if (!inherited::net_Spawn(DC)) return FALSE;
	return TRUE;
}

void CInventoryContainer::net_Destroy()
{
	inherited::net_Destroy();
}

float CInventoryContainer::Weight() const
{
	float res = inherited::Weight();

	xr_vector<u16>::const_iterator I = m_items.begin();
	xr_vector<u16>::const_iterator E = m_items.end();
	for (; I != E; ++I)
	{
		CInventoryItem* itm = smart_cast<CInventoryItem*>(Level().Objects.net_Find(*I));
		if (itm)
			res += itm->Weight();
	}
	return res;
}

void CInventoryContainer::RecalcOwnerWeight()
{
	// m_pInventory is set while this item sits in someone's inventory
	// (the actor's, per the V1 constraint) and null on the ground,
	// where nobody is carrying the weight anyway.
	if (m_pInventory)
	{
		m_pInventory->InvalidateState();
		m_pInventory->CalcTotalWeight();
	}
}

////////////////////////////////////////////////////////////////////////////
// script registration, box-style
////////////////////////////////////////////////////////////////////////////

using namespace luabind;

#pragma optimize("s",on)
void CInventoryContainer::script_register(lua_State* L)
{
	module(L)
		[
			class_<CInventoryContainer, CGameObject>("CInventoryContainer")
			.def(constructor<>())
		];
}
