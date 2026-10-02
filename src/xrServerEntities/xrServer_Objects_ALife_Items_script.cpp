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

int sqa_inventory_layout_version() { return 1; }
int sqa_rig_transfer_version() { return 1; }
int sqa_rig_membership_version() { return 1; }
int sqa_rig_pouches_version() { return 1; }

using namespace luabind;

#pragma optimize("s",on)
void CSE_ALifeInventoryItem::script_register(lua_State* L)
{
	module(L)[
        def("sqa_inventory_layout_version", &sqa_inventory_layout_version),
        def("sqa_rig_transfer_version", &sqa_rig_transfer_version),
        def("sqa_rig_membership_version", &sqa_rig_membership_version),
        def("sqa_rig_pouches_version", &sqa_rig_pouches_version),
        class_<inventory_layout::Placement>("sqa_inventory_placement")
        .def_readonly("valid", &inventory_layout::Placement::valid)
        .def_readonly("x", &inventory_layout::Placement::x)
        .def_readonly("y", &inventory_layout::Placement::y)
        .def_readonly("width", &inventory_layout::Placement::width)
        .def_readonly("height", &inventory_layout::Placement::height)
        .def_readonly("rotated", &inventory_layout::Placement::rotated)
        .def_readonly("manual", &inventory_layout::Placement::manual),
        class_<inventory_membership::Membership>("sqa_rig_membership_record")
        .def("valid", &inventory_membership::Membership::valid)
        .def("key", &inventory_membership::Membership::key)
        .def_readonly("layout", &inventory_membership::Membership::layout)
        .def_readonly("order", &inventory_membership::Membership::order),
		class_<CSE_ALifeInventoryItem>
		("cse_alife_inventory_item")
		//			.def(		constructor<LPCSTR>())
        .def("rig_pouches_initialized", &CSE_ALifeInventoryItem::rig_pouches_initialized)
        .def("rig_pouch", &CSE_ALifeInventoryItem::rig_pouch)
        .def("set_rig_pouch", &CSE_ALifeInventoryItem::set_rig_pouch)
        .def("set_rig_pouches", &CSE_ALifeInventoryItem::set_rig_pouches)
        .def("rig_membership", &CSE_ALifeInventoryItem::rig_membership)
        .def("rig_membership_matches", &CSE_ALifeInventoryItem::rig_membership_matches)
        .def("set_rig_membership", &CSE_ALifeInventoryItem::set_rig_membership)
        .def("clear_rig_membership", &CSE_ALifeInventoryItem::clear_rig_membership)
        .def("inventory_layout", &CSE_ALifeInventoryItem::inventory_layout)
        .def("set_inventory_layout", &CSE_ALifeInventoryItem::set_inventory_layout)
        .def("clear_inventory_layout", &CSE_ALifeInventoryItem::clear_inventory_layout)
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
