#pragma once

#include "InventoryContainer.h"

// The normal II_BPACK item keeps its slot/upgrade behavior while gaining
// native child ownership. A bag's contents can then travel with that exact
// backpack object after it is unequipped or moved into a stash.
class CBackpack : public CInventoryContainer
{
private:
	typedef CInventoryContainer inherited;
public:
	CBackpack();
	virtual ~CBackpack();

	virtual void Load(LPCSTR section);

	virtual void Hit(float P, ALife::EHitType hit_type);

	virtual void OnMoveToSlot(const SInvItemPlace& prev);
	virtual void OnMoveToRuck(const SInvItemPlace& previous_place);
	virtual void OnH_A_Chield();

	float m_additional_weight;
	float m_additional_weight2;
	float m_fPowerRestoreSpeed;
	float m_fPowerLoss;

	virtual BOOL net_Spawn(CSE_Abstract* DC);
	virtual void net_Export(NET_Packet& P);
	virtual void net_Import(NET_Packet& P);

protected:
	virtual bool install_upgrade_impl(LPCSTR section, bool test);
};
