////////////////////////////////////////////////////////////////////////////
//	Module 		: xrServer_Objects_ALife.h
//	Created 	: 19.09.2002
//  Modified 	: 04.06.2003
//	Author		: Oles Shyshkovtsov, Alexander Maksimchuk, Victor Reutskiy and Dmitriy Iassenev
//	Description : Server objects items for ALife simulator
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "xrServer_Objects_ALife.h"
#include "PHSynchronize.h"
#include "inventory_space.h"

#include "character_info_defs.h"
#include "infoportiondefs.h"

#pragma warning(push)
#pragma warning(disable:4005)

class CSE_ALifeItemAmmo;

SERVER_ENTITY_DECLARE_BEGIN0(CSE_ALifeInventoryItem)
public:
	enum
	{
		inventory_item_state_enabled = u8(1) << 0,
		inventory_item_angular_null = u8(1) << 1,
		inventory_item_linear_null = u8(1) << 2,
	};

	union mask_num_items
	{
		struct
		{
			u8 num_items : 5;
			u8 mask : 3;
		};

		u8 common;
	};

public:
	float m_fCondition;
	float m_fMass;
	u32 m_dwCost;
	s32 m_iHealthValue;
	s32 m_iFoodValue;
	float m_fDeteriorationValue;
	CSE_ALifeObject* m_self;
	u32 m_last_update_time;
	xr_vector<shared_str> m_upgrades;

	// ============================================================
	//  ITEM DATA: a small store a script may keep ON the item
	//
	//  Saved with the item, so it survives the things an object id does
	//  not: going offline, being carried to another level, sitting in a
	//  stash or inside a container. A mod that files its bookkeeping by
	//  id has to cope with that id being handed to something else later;
	//  a mod that keeps it here does not, because the data goes wherever
	//  the item goes.
	//
	//  A VECTOR OF PAIRS, not a map. A handful of keys per item is a
	//  linear scan either way, the order is stable so the same state
	//  saves as the same bytes, and object_saver/object_loader already
	//  serialise a container of std::pair<shared_str, shared_str> with no
	//  help at all - which is what keeps STATE_Write to one line.
	// ============================================================
	typedef std::pair<shared_str, shared_str> item_data_pair;
	typedef xr_vector<item_data_pair> item_data_store;
	item_data_store m_item_data;

public:
	//  LIMITS, because a store with no ceiling is a save file with no
	//  ceiling. The whole of an object's state has to fit ONE NET_Packet,
	//  and that is 16 KB for everything the object has to say - so the
	//  budget here is a quarter of it and set_data refuses rather than
	//  building a packet that cannot be sent. Measured on the SERIALISED
	//  length, which is the number that actually decides.
	enum
	{
		item_data_max_key = 63,
		item_data_max_value = 2047,
		item_data_max_keys = 32,
		item_data_max_bytes = 4096,
	};

	//  ABSENT AND EMPTY ARE DIFFERENT ANSWERS, so there are two calls
	//  rather than one that overloads "" to mean both. That exact
	//  conflation has cost this pack a bug already.
	bool has_data(LPCSTR key) const;
	LPCSTR get_data(LPCSTR key) const;
	//  false when refused - key too long, too many keys, over budget -
	//  so a caller is never left believing it stored something.
	bool set_data(LPCSTR key, LPCSTR value);
	bool remove_data(LPCSTR key);
	void clear_data();
	//  For walking what is there: a migration, a debug dump, a mod
	//  tidying up after an older version of itself.
	u32 data_count() const;
	LPCSTR data_key(u32 index) const;
	//  What the store costs in the packet as it stands.
	u32 data_bytes() const;

private:
	const item_data_pair* find_data(LPCSTR key) const;

public:
	CSE_ALifeInventoryItem(LPCSTR caSection);
	virtual ~CSE_ALifeInventoryItem();
	// we need this to prevent virtual inheritance :-(
	virtual CSE_Abstract* base() = 0;
	virtual const CSE_Abstract* base() const = 0;
	virtual CSE_Abstract* init();
	virtual CSE_Abstract* cast_abstract() { return 0; };
	virtual CSE_ALifeInventoryItem* cast_inventory_item() { return this; };
	virtual u32 update_rate() const;
	virtual BOOL Net_Relevant();

	bool has_upgrade(const shared_str& upgrade_id);
	void add_upgrade(const shared_str& upgrade_id);

private:
	bool prev_freezed;
	bool freezed;
	u32 m_freeze_time;
	static const u32 m_freeze_delta_time;
	static const u32 random_limit;
	CRandom m_relevent_random;

public:
	// end of the virtual inheritance dependant code

	IC bool attached() const
	{
		return (base()->ID_Parent < 0xffff);
	}

	virtual bool bfUseful();

	/////////// network ///////////////
	u8 m_u8NumItems;
	SPHNetState State;
	///////////////////////////////////
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN2(CSE_ALifeItem, CSE_ALifeDynamicObjectVisual, CSE_ALifeInventoryItem)
	bool m_physics_disabled;

	CSE_ALifeItem(LPCSTR caSection);
	virtual ~CSE_ALifeItem();
	virtual CSE_Abstract* base();
	virtual const CSE_Abstract* base() const;
	virtual CSE_Abstract* init();
	virtual CSE_Abstract* cast_abstract() { return this; };
	virtual CSE_ALifeInventoryItem* cast_inventory_item() { return this; };
	virtual BOOL Net_Relevant();
	virtual void OnEvent(NET_Packet& tNetPacket, u16 type, u32 time, ClientID sender);
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemTorch, CSE_ALifeItem)

	//флаги
	enum EStats
	{
		eTorchActive = (1 << 0),
		eNightVisionActive = (1 << 1),
		eAttached = (1 << 2)
	};

	bool m_active;
	bool m_nightvision_active;
	bool m_attached;
	CSE_ALifeItemTorch(LPCSTR caSection);
	virtual ~CSE_ALifeItemTorch();
	virtual BOOL Net_Relevant();

SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemAmmo, CSE_ALifeItem)
	u16 a_elapsed;
	u16 m_boxSize;

	CSE_ALifeItemAmmo(LPCSTR caSection);
	virtual ~CSE_ALifeItemAmmo();
	virtual CSE_ALifeItemAmmo* cast_item_ammo() { return this; };
	virtual bool can_switch_online() const;
	virtual bool can_switch_offline() const;
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemWeapon, CSE_ALifeItem)

	typedef ALife::EWeaponAddonStatus EWeaponAddonStatus;

	//текущее состояние аддонов
	enum EWeaponAddonState
	{
		eWeaponAddonScope = 0x01,
		eWeaponAddonGrenadeLauncher = 0x02,
		eWeaponAddonSilencer = 0x04
	};

	EWeaponAddonStatus m_scope_status;
	EWeaponAddonStatus m_silencer_status;
	EWeaponAddonStatus m_grenade_launcher_status;

	u32 timestamp;
	u8 wpn_flags;
	u8 wpn_state;
	u8 ammo_type;
	u16 a_current;
	u16 a_elapsed;

	//count of grenades to spawn in grenade launcher [ttcccccc]
	//WARNING! hight 2 bits (tt bits) indicate type of grenade, so maximum grenade count is 2^6 = 64
	struct grenade_count_t
	{
		u8 grenades_count : 6;
		u8 grenades_type : 2;

		u8 pack_to_byte() const
		{
			return (grenades_type << 6) | grenades_count;
		}

		void unpack_from_byte(u8 const b)
		{
			grenades_type = (b >> 6);
			grenades_count = b & 0x3f; //111111
		}
	}; //struct grenade_count_t
	grenade_count_t a_elapsed_grenades;

	float m_fHitPower;
	ALife::EHitType m_tHitType;
	LPCSTR m_caAmmoSections;
	u32 m_dwAmmoAvailable;
	Flags8 m_addon_flags;
	u8 m_bZoom;
	u32 m_ef_main_weapon_type;
	u32 m_ef_weapon_type;

	CSE_ALifeItemWeapon(LPCSTR caSection);
	virtual ~CSE_ALifeItemWeapon();
	virtual void OnEvent(NET_Packet& P, u16 type, u32 time, ClientID sender);
	virtual u32 ef_main_weapon_type() const;
	virtual u32 ef_weapon_type() const;
	u8 get_slot();
	u16 get_ammo_limit();
	u16 get_ammo_total();
	u16 get_ammo_elapsed();
	void set_ammo_elapsed(u16 count);
	u16 get_ammo_magsize();
	void clone_addons(CSE_ALifeItemWeapon* parent);

	void clone_upgrades(CSE_ALifeItemWeapon* parent);

	virtual BOOL Net_Relevant();

	virtual CSE_ALifeItemWeapon* cast_item_weapon() { return this; }
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemWeaponMagazined, CSE_ALifeItemWeapon)
	u8 m_u8CurFireMode;
	CSE_ALifeItemWeaponMagazined(LPCSTR caSection);
	virtual ~CSE_ALifeItemWeaponMagazined();

	virtual CSE_ALifeItemWeapon* cast_item_weapon() { return this; }
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemWeaponMagazinedWGL, CSE_ALifeItemWeaponMagazined)
	bool m_bGrenadeMode;
	CSE_ALifeItemWeaponMagazinedWGL(LPCSTR caSection);
	virtual ~CSE_ALifeItemWeaponMagazinedWGL();

	virtual CSE_ALifeItemWeapon* cast_item_weapon() { return this; }
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemWeaponShotGun, CSE_ALifeItemWeaponMagazined)
	xr_vector<u8> m_AmmoIDs;
	CSE_ALifeItemWeaponShotGun(LPCSTR caSection);
	virtual ~CSE_ALifeItemWeaponShotGun();

	virtual CSE_ALifeItemWeapon* cast_item_weapon() { return this; }
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemWeaponAutoShotGun, CSE_ALifeItemWeaponShotGun)
	CSE_ALifeItemWeaponAutoShotGun(LPCSTR caSection);
	virtual ~CSE_ALifeItemWeaponAutoShotGun();

	virtual CSE_ALifeItemWeapon* cast_item_weapon() { return this; }
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemDetector, CSE_ALifeItem)
	u32 m_ef_detector_type;
	CSE_ALifeItemDetector(LPCSTR caSection);
	virtual ~CSE_ALifeItemDetector();
	virtual u32 ef_detector_type() const;
	virtual CSE_ALifeItemDetector* cast_item_detector() { return this; }
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemArtefact, CSE_ALifeItem)
	float m_fAnomalyValue;
	CSE_ALifeItemArtefact(LPCSTR caSection);
	virtual ~CSE_ALifeItemArtefact();
	virtual BOOL Net_Relevant();
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemPDA, CSE_ALifeItem)
	u16 m_original_owner;
	shared_str m_specific_character;
	shared_str m_info_portion;

	CSE_ALifeItemPDA(LPCSTR caSection);
	virtual ~CSE_ALifeItemPDA();
	virtual CSE_ALifeItemPDA* cast_item_pda() { return this; };
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemDocument, CSE_ALifeItem)
	shared_str m_wDoc;
	CSE_ALifeItemDocument(LPCSTR caSection);
	virtual ~CSE_ALifeItemDocument();
SERVER_ENTITY_DECLARE_END

// AMP: the server half of the carryable container (CInventoryContainer).
// A CSE_ALifeItem so it can be carried, with the inventory box's
// online/offline handling so a container lying on the ground far from
// the actor does not lose its contents to the one-level default.
SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemContainer, CSE_ALifeItem)
	CSE_ALifeItemContainer(LPCSTR caSection);
	virtual ~CSE_ALifeItemContainer();
#ifdef XRGAME_EXPORTS
	virtual void add_offline(const xr_vector<ALife::_OBJECT_ID>& saved_children, const bool& update_registries);
	virtual void add_online(const bool& update_registries);
#endif
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemGrenade, CSE_ALifeItem)
	u32 m_ef_weapon_type;
	CSE_ALifeItemGrenade(LPCSTR caSection);
	virtual ~CSE_ALifeItemGrenade();
	virtual u32 ef_weapon_type() const;
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemExplosive, CSE_ALifeItem)
	CSE_ALifeItemExplosive(LPCSTR caSection);
	virtual ~CSE_ALifeItemExplosive();
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemBolt, CSE_ALifeItem)
	u32 m_ef_weapon_type;
	CSE_ALifeItemBolt(LPCSTR caSection);
	virtual ~CSE_ALifeItemBolt();
	virtual bool can_save() const;
	virtual bool used_ai_locations() const;
	virtual u32 ef_weapon_type() const;
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemCustomOutfit, CSE_ALifeItem)
	u32 m_ef_equipment_type;
	CSE_ALifeItemCustomOutfit(LPCSTR caSection);
	virtual ~CSE_ALifeItemCustomOutfit();
	virtual u32 ef_equipment_type() const;
	virtual BOOL Net_Relevant();
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_ALifeItemHelmet, CSE_ALifeItem)
	CSE_ALifeItemHelmet(LPCSTR caSection);
	virtual ~CSE_ALifeItemHelmet();
	virtual BOOL Net_Relevant();
SERVER_ENTITY_DECLARE_END

#pragma warning(pop)
