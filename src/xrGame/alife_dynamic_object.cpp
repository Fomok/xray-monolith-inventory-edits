////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_dynamic_object.cpp
//	Created 	: 27.10.2005
//  Modified 	: 27.10.2005
//	Author		: Dmitriy Iassenev
//	Description : ALife dynamic object class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrServer_Objects_ALife.h"
#include "xrServer_Objects_ALife_Items.h" // AMP: CSE_ALifeItemContainer
#include "alife_simulator.h"
#include "alife_schedule_registry.h"
#include "alife_graph_registry.h"
#include "alife_object_registry.h"
#include "level_graph.h"
#include "game_level_cross_table.h"
#include "game_graph.h"
#include "xrServer.h"
#include "level.h"
#include "map_manager.h"

void CSE_ALifeDynamicObject::on_spawn()
{
#ifdef DEBUG
	//	Msg			("[LSS] spawning object [%d][%d][%s][%s]",ID,ID_Parent,name(),name_replace());
#endif
}

void CSE_ALifeDynamicObject::on_register()
{
	CSE_ALifeObject* object = this;
	while (object->ID_Parent != ALife::_OBJECT_ID(-1))
	{
		object = ai().alife().objects().object(object->ID_Parent);
		VERIFY(object);
	}

	if (!alife().graph().level().object(object->ID, true))
		clear_client_data();

    ::luabind::functor<void> funct;
    if (ai().script_engine().functor("_G.CSE_ALifeDynamicObject_on_register", funct))
        funct((u16)ID);
}

void CSE_ALifeDynamicObject::on_before_register()
{
}

void CSE_ALifeDynamicObject::on_unregister()
{
	::luabind::functor<void> funct;
	if (ai().script_engine().functor("_G.CSE_ALifeDynamicObject_on_unregister", funct))
		funct((u16)ID);
	Level().MapManager().OnObjectDestroyNotify(ID);
}

void CSE_ALifeDynamicObject::switch_online()
{
	R_ASSERT(!m_bOnline);
	m_bOnline = true;
	alife().add_online(this);
}

void CSE_ALifeDynamicObject::switch_offline()
{
	R_ASSERT(m_bOnline);
	m_bOnline = false;
	alife().remove_online(this);

	clear_client_data();
}

void CSE_ALifeDynamicObject::add_online(const bool& update_registries)
{
	if (!update_registries)
		return;

	alife().scheduled().remove(this);
	alife().graph().remove(this, m_tGraphID, false);
}

void CSE_ALifeDynamicObject::add_offline(const xr_vector<ALife::_OBJECT_ID>& saved_children,
                                         const bool& update_registries)
{
	if (!update_registries)
		return;

	alife().scheduled().add(this);
	alife().graph().add(this, m_tGraphID, false);
}

bool CSE_ALifeDynamicObject::synchronize_location()
{
	if (!ai().level_graph().valid_vertex_id(m_tNodeID)) return false;

	if (!ai().level_graph().valid_vertex_position(o_Position) || ai().level_graph().inside(
		ai().level_graph().vertex(m_tNodeID),
		o_Position))
		return (true);

	u32 const new_vertex_id = ai().level_graph().vertex(m_tNodeID, o_Position);
	if (!m_bOnline && !ai().level_graph().inside(new_vertex_id, o_Position))
		return (true);

	m_tNodeID = new_vertex_id;
	GameGraph::_GRAPH_ID tGraphID = ai().cross_table().vertex(m_tNodeID).game_vertex_id();
	if (tGraphID != m_tGraphID)
	{
		if (!m_bOnline)
		{
			Fvector position = o_Position;
			u32 level_vertex_id = m_tNodeID;
			alife().graph().change(this, m_tGraphID, tGraphID);
			if (ai().level_graph().inside(ai().level_graph().vertex(level_vertex_id), position))
			{
				level_vertex_id = m_tNodeID;
				o_Position = position;
			}
		}
		else
		{
			VERIFY(ai().game_graph().vertex(tGraphID)->level_id() == alife().graph().level().level_id());
			m_tGraphID = tGraphID;
		}
	}

	m_fDistance = ai().cross_table().vertex(m_tNodeID).distance();

	return (true);
}

void CSE_ALifeDynamicObject::try_switch_online()
{
	CSE_ALifeSchedulable* schedulable = smart_cast<CSE_ALifeSchedulable*>(this);
	// checking if the abstract monster has just died
	if (schedulable)
	{
		if (!schedulable->need_update(this))
		{
			if (alife().scheduled().object(ID, true))
				alife().scheduled().remove(this);
		}
		else if (!alife().scheduled().object(ID, true))
			alife().scheduled().add(this);
	}

	if (!can_switch_online())
	{
		on_failed_switch_online();
		return;
	}

	if (!can_switch_offline())
	{
		alife().switch_online(this);
		return;
	}

	if (alife().graph().actor()->o_Position.distance_to(o_Position) > alife().online_distance())
	{
		on_failed_switch_online();
		return;
	}

	alife().switch_online(this);
}

void CSE_ALifeDynamicObject::try_switch_offline()
{
	if (!can_switch_offline())
		return;

	if (!can_switch_online())
	{
		alife().switch_offline(this);
		return;
	}

	if (alife().graph().actor()->o_Position.distance_to(o_Position) <= alife().offline_distance())
		return;

	alife().switch_offline(this);
}

bool CSE_ALifeDynamicObject::redundant() const
{
	return (false);
}

/// ---------------------------- CSE_ALifeInventoryBox ---------------------------------------------

void CSE_ALifeInventoryBox::add_online(const bool& update_registries)
{
	CSE_ALifeDynamicObjectVisual* object = (this);

	NET_Packet tNetPacket;
	ClientID clientID;
	clientID.set(
		object->alife().server().GetServerClient() ? object->alife().server().GetServerClient()->ID.value() : 0);

	ALife::OBJECT_IT I = object->children.begin();
	ALife::OBJECT_IT E = object->children.end();
	for (; I != E; ++I)
	{
		CSE_ALifeDynamicObject* l_tpALifeDynamicObject = ai().alife().objects().object(*I);
		CSE_ALifeInventoryItem* l_tpALifeInventoryItem = smart_cast<CSE_ALifeInventoryItem*>(l_tpALifeDynamicObject);
		R_ASSERT2(l_tpALifeInventoryItem, "Non inventory item object has parent?!");
		l_tpALifeInventoryItem->base()->s_flags.or(M_SPAWN_UPDATE);
		CSE_Abstract* l_tpAbstract = smart_cast<CSE_Abstract*>(l_tpALifeInventoryItem);
		object->alife().server().entity_Destroy(l_tpAbstract);

#ifdef DEBUG
		//		if (psAI_Flags.test(aiALife))
//			Msg					("[LSS] Spawning item [%s][%s][%d]",l_tpALifeInventoryItem->base()->name_replace(),*l_tpALifeInventoryItem->base()->s_name,l_tpALifeDynamicObject->ID);
		Msg						(
			"[LSS][%d] Going online [%d][%s][%d] with parent [%d][%s] on '%s'",
			Device.dwFrame,
			Device.dwTimeGlobal,
			l_tpALifeInventoryItem->base()->name_replace(),
			l_tpALifeInventoryItem->base()->ID,
			ID,
			name_replace(),
			"*SERVER*"
		);
#endif

		l_tpALifeDynamicObject->o_Position = object->o_Position;
		l_tpALifeDynamicObject->m_tNodeID = object->m_tNodeID;
		object->alife().server().Process_spawn(tNetPacket, clientID,FALSE, l_tpALifeInventoryItem->base());
		l_tpALifeDynamicObject->s_flags.and(u16(-1) ^ M_SPAWN_UPDATE);
		l_tpALifeDynamicObject->m_bOnline = true;
	}

	CSE_ALifeDynamicObjectVisual::add_online(update_registries);
}

void CSE_ALifeInventoryBox::add_offline(const xr_vector<ALife::_OBJECT_ID>& saved_children,
                                        const bool& update_registries)
{
	CSE_ALifeDynamicObjectVisual* object = (this);

	for (u32 i = 0, n = saved_children.size(); i < n; ++i)
	{
		CSE_ALifeDynamicObject* child = smart_cast<CSE_ALifeDynamicObject*>(
			ai().alife().objects().object(saved_children[i], true));
		// R_ASSERT(child);
		if (!child)
		{
			Msg("[DO] can't switch child [%d] offline, it's null", saved_children[i]);
			continue;
		}
		child->m_bOnline = false;

		CSE_ALifeInventoryItem* inventory_item = smart_cast<CSE_ALifeInventoryItem*>(child);
		VERIFY2(inventory_item, "Non inventory item object has parent?!");
#ifdef DEBUG
		//		if (psAI_Flags.test(aiALife))
//			Msg					("[LSS] Destroying item [%s][%s][%d]",inventory_item->base()->name_replace(),*inventory_item->base()->s_name,inventory_item->base()->ID);
		Msg						(
			"[LSS][%d] Going offline [%d][%s][%d] with parent [%d][%s] on '%s'",
			Device.dwFrame,
			Device.dwTimeGlobal,
			inventory_item->base()->name_replace(),
			inventory_item->base()->ID,
			ID,
			name_replace(),
			"*SERVER*"
		);
#endif

		ALife::_OBJECT_ID item_id = inventory_item->base()->ID;
		inventory_item->base()->ID = object->alife().server().PerformIDgen(item_id);

		if (!child->can_save())
		{
			object->alife().release(child);
			--i;
			--n;
			continue;
		}
		child->clear_client_data();
		object->alife().graph().add(child, child->m_tGraphID, false);
		//		object->alife().graph().attach	(*object,inventory_item,child->m_tGraphID,true);
		alife().graph().remove(child, child->m_tGraphID);
		children.push_back(child->ID);
		child->ID_Parent = ID;
	}


	CSE_ALifeDynamicObjectVisual::add_offline(saved_children, update_registries);
}

////////////////////////////////////////////////////////////////////////////
// AMP: CSE_ALifeItemContainer - the carryable container's switch
// handling, copied from the inventory box above, which is the engine's
// one proven owner of items through the online/offline boundary.
//
// Two deliberate differences from the box's code:
//   * the !can_save() branch uses `continue` (the box's `--i; --n;`
//     mutates the bounds of a loop over a vector it does not own and
//     skips the child after every release);
//   * no visual asserts - a container is a plain item.
//
// These run when the container itself crosses the boundary as a ROOT
// object - dropped on the ground and the actor walks away.
//
// A container carried in somebody's inventory crosses it a different
// way: the OWNER switches, and the switch manager saves one level of
// children and lets Perform_destroy wipe the rest. That was the bug
// behind an emptied container after a level change. The fix is in
// CALifeSwitchManager::remove_online (deep save) and add_online_impl in
// alife_trader_abstract.cpp (deep spawn), which call the two functions
// below for the carried case as well.
////////////////////////////////////////////////////////////////////////////
void CSE_ALifeItemContainer::add_online(const bool& update_registries)
{
	NET_Packet tNetPacket;
	ClientID clientID;
	clientID.set(
		alife().server().GetServerClient() ? alife().server().GetServerClient()->ID.value() : 0);

	// DIAG: what the container believes it is carrying at the moment it
	// comes back into the world.
	Msg("[AMP-S] container %d add_online with %d child(ren)", ID, (int)children.size());

	// A list of ids is a promise that each one still names something.
	// Keep only the ones that do: an id left behind by an earlier fault
	// would otherwise sit in children until xrServer::Perform_destroy
	// walks it at shutdown and asserts "child registered but not found".
	// Losing an item is bad; crashing the game on the way out is worse.
	ALife::OBJECT_VECTOR survivors;
	survivors.reserve(children.size());

	ALife::OBJECT_IT I = children.begin();
	ALife::OBJECT_IT E = children.end();
	for (; I != E; ++I)
	{
		CSE_ALifeDynamicObject* child = ai().alife().objects().object(*I, true);
		CSE_ALifeInventoryItem* item = child ? smart_cast<CSE_ALifeInventoryItem*>(child) : NULL;
		if (!item)
		{
			Msg("[AMP-S] container [%d]: child [%d] IS NOT IN THE REGISTRY - dropping the id", ID, *I);
			continue;
		}
		Msg("[AMP-S]   spawning child %d back", *I);

		item->base()->s_flags.or(M_SPAWN_UPDATE);
		CSE_Abstract* abstract = smart_cast<CSE_Abstract*>(item);
		alife().server().entity_Destroy(abstract);

		child->o_Position = o_Position;
		child->m_tNodeID = m_tNodeID;
		alife().server().Process_spawn(tNetPacket, clientID, FALSE, item->base());
		child->s_flags.and(u16(-1) ^ M_SPAWN_UPDATE);
		child->m_bOnline = true;
		survivors.push_back(child->ID);

		// a container inside a container - not allowed in V1, but the
		// recursion costs a branch and stops it being a silent loss.
		if (!child->children.empty())
			child->add_online(false);
	}

	children.swap(survivors);
	Msg("[AMP-S] container %d came online holding %d", ID, (int)children.size());

	CSE_ALifeItem::add_online(update_registries);
}

void CSE_ALifeItemContainer::add_offline(const xr_vector<ALife::_OBJECT_ID>& saved_children,
                                         const bool& update_registries)
{
	// DIAG: what the switch manager handed us on the way out. If this says
	// nought, the contents were gone before this ran.
	Msg("[AMP-S] container %d add_offline with %d saved child(ren)", ID,
	    (int)saved_children.size());

	for (u32 i = 0, n = saved_children.size(); i < n; ++i)
	{
		CSE_ALifeDynamicObject* child = smart_cast<CSE_ALifeDynamicObject*>(
			ai().alife().objects().object(saved_children[i], true));
		if (!child)
		{
			Msg("[AMP-S]   child %d IS NULL on the way offline - lost here", saved_children[i]);
			continue;
		}
		child->m_bOnline = false;

		CSE_ALifeInventoryItem* item = smart_cast<CSE_ALifeInventoryItem*>(child);
		VERIFY2(item, "Non inventory item object inside a container?!");
		if (!item)
			continue;

		ALife::_OBJECT_ID item_id = item->base()->ID;
		item->base()->ID = alife().server().PerformIDgen(item_id);

		if (!child->can_save())
		{
			Msg("[AMP-S]   child %d cannot be saved - released", child->ID);
			alife().release(child);
			continue;
		}
		child->clear_client_data();
		alife().graph().add(child, child->m_tGraphID, false);
		alife().graph().remove(child, child->m_tGraphID);
		children.push_back(child->ID);
		child->ID_Parent = ID;
		Msg("[AMP-S]   child kept, new id %d, parent %d", child->ID, ID);
	}

	Msg("[AMP-S] container %d went offline holding %d", ID, (int)children.size());

	CSE_ALifeItem::add_offline(saved_children, update_registries);
}

void CSE_ALifeDynamicObject::clear_client_data()
{
#ifdef DEBUG
	if (!client_data.empty())
		Msg						("CSE_ALifeDynamicObject::switch_offline: client_data is cleared for [%d][%s]",ID,name_replace());
#endif // DEBUG
	if (!keep_saved_data_anyway())
		client_data.clear();
}

void CSE_ALifeDynamicObject::on_failed_switch_online()
{
	clear_client_data();
}
