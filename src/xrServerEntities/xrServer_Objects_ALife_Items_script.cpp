////////////////////////////////////////////////////////////////////////////
//	Module 		: xrServer_Objects_ALife_Items_script.cpp
//	Created 	: 19.09.2002
//  Modified 	: 04.06.2003
//	Author		: Dmitriy Iassenev
//	Description : Server items for ALife simulator, script export
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "xrServer_Objects_ALife_Items.h"
#include "xrServer_script_macroses.h"

void add_upgrade_script(CSE_ALifeInventoryItem* ta, LPCSTR str)
{
	ta->add_upgrade(str);
}

bool has_upgrade_script(CSE_ALifeInventoryItem* ta, LPCSTR str)
{
	return ta->has_upgrade(str);
}

// ============================================================
//  ITEM DATA, from Lua
//
//  Thin wrappers on purpose: every rule about what may be stored lives in
//  one place - CSE_ALifeInventoryItem::set_data - so a script and the engine
//  cannot end up with different ideas about what fitted.
//
//  A nil from Lua arrives here as a NULL LPCSTR, and every call below is
//  written to answer rather than crash - see the note on find_data.
// ============================================================
bool item_has_data_script(CSE_ALifeInventoryItem* ta, LPCSTR key)
{
	return ta->has_data(key);
}

LPCSTR item_get_data_script(CSE_ALifeInventoryItem* ta, LPCSTR key)
{
	return ta->get_data(key);
}

bool item_set_data_script(CSE_ALifeInventoryItem* ta, LPCSTR key, LPCSTR value)
{
	return ta->set_data(key, value);
}

bool item_remove_data_script(CSE_ALifeInventoryItem* ta, LPCSTR key)
{
	return ta->remove_data(key);
}

void item_clear_data_script(CSE_ALifeInventoryItem* ta)
{
	ta->clear_data();
}

u32 item_data_count_script(CSE_ALifeInventoryItem* ta)
{
	return ta->data_count();
}

//  ONE-BASED, because it is Lua that walks this. An engine index that runs
//  0..n-1 inside a language whose every other loop runs 1..n is a fencepost
//  bug waiting for somebody, so the seam is here where it can be written
//  down rather than in every script.
LPCSTR item_data_key_script(CSE_ALifeInventoryItem* ta, u32 index)
{
	if (!index)
		return "";

	return ta->data_key(index - 1);
}

u32 item_data_bytes_script(CSE_ALifeInventoryItem* ta)
{
	return ta->data_bytes();
}

using namespace luabind;

#pragma optimize("s",on)
void CSE_ALifeInventoryItem::script_register(lua_State* L)
{
	module(L)[
		class_<CSE_ALifeInventoryItem>
		("cse_alife_inventory_item")
		//			.def(		constructor<LPCSTR>())
		.def("has_upgrade", &has_upgrade)
		.def("add_upgrade", &add_upgrade)

		//  A small store kept on the item and saved with it. get_data
		//  answers "" for a key that is not there, so has_data is what
		//  distinguishes absent from empty; set_data answers false when
		//  it refused, and says why in the log.
		.def("has_data", &item_has_data_script)
		.def("get_data", &item_get_data_script)
		.def("set_data", &item_set_data_script)
		.def("remove_data", &item_remove_data_script)
		.def("clear_data", &item_clear_data_script)
		.def("data_count", &item_data_count_script)
		.def("data_key", &item_data_key_script)
		.def("data_bytes", &item_data_bytes_script)
	];
}

void CSE_ALifeItem::script_register(lua_State* L)
{
	module(L)[
		luabind_class_item2(
			//		luabind_class_abstract2(
			CSE_ALifeItem,
			"cse_alife_item",
			CSE_ALifeDynamicObjectVisual,
			CSE_ALifeInventoryItem
		)
	];
}

void CSE_ALifeItemTorch::script_register(lua_State* L)
{
	module(L)[
		luabind_class_item1(
			CSE_ALifeItemTorch,
			"cse_alife_item_torch",
			CSE_ALifeItem
		)
	];
}

void CSE_ALifeItemAmmo::script_register(lua_State* L)
{
	module(L)[
		luabind_class_item1(
			CSE_ALifeItemAmmo,
			"cse_alife_item_ammo",
			CSE_ALifeItem
		)
	];
}

void CSE_ALifeItemWeapon::script_register(lua_State* L)
{
	module(L)[
		luabind_class_item1(
			CSE_ALifeItemWeapon,
			"cse_alife_item_weapon",
			CSE_ALifeItem
		)
		.def("clone_addons", &CSE_ALifeItemWeapon::clone_addons)
		.def("clone_upgrades", &CSE_ALifeItemWeapon::clone_upgrades)
		.def("set_ammo_elapsed", &CSE_ALifeItemWeapon::set_ammo_elapsed)
		.def("get_ammo_elapsed", &CSE_ALifeItemWeapon::get_ammo_elapsed)
		.def("get_ammo_magsize", &CSE_ALifeItemWeapon::get_ammo_magsize)
	];
}

void CSE_ALifeItemWeaponShotGun::script_register(lua_State* L)
{
	module(L)[
		luabind_class_item1(
			CSE_ALifeItemWeaponShotGun,
			"cse_alife_item_weapon_shotgun",
			CSE_ALifeItemWeapon
		)
	];
}

void CSE_ALifeItemWeaponAutoShotGun::script_register(lua_State* L)
{
	module(L)[
		luabind_class_item1(
			CSE_ALifeItemWeaponAutoShotGun,
			"cse_alife_item_weapon_auto_shotgun",
			CSE_ALifeItemWeapon
		)
	];
}

void CSE_ALifeItemDetector::script_register(lua_State* L)
{
	module(L)[
		luabind_class_item1(
			CSE_ALifeItemDetector,
			"cse_alife_item_detector",
			CSE_ALifeItem
		)
	];
}

void CSE_ALifeItemArtefact::script_register(lua_State* L)
{
	module(L)[
		luabind_class_item1(
			CSE_ALifeItemArtefact,
			"cse_alife_item_artefact",
			CSE_ALifeItem
		)
	];
}
