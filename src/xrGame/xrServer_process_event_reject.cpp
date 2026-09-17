#include "stdafx.h"
#include "xrserver.h"
#include "xrserver_objects.h"

// ============================================================
// AMP: WHO IS REALLY HOLDING IT
//
// A carried container owns its contents: the actor's children list
// names the case, and the case's children list names what is inside.
// Everything that takes an item off the actor - a quest hand-in
// releasing it, a script dropping it, a trade - asks the server to
// detach it from the ACTOR, because until containers there was nowhere
// else for one of the actor's items to be.
//
// That ask used to fail silently: the WARNING below, then `return
// false`, and the caller carries on as if it had worked. For a fetch
// task that is the reward paid and the artefact still in the case.
//
// So when the claimed holder does not have the item, but one of its own
// children does, that child is the holder and the detach happens there.
// One level deep; nothing else in this engine nests. This only runs
// where the old code was about to refuse outright, so a request that
// used to work still takes exactly the path it took before - a loose
// item is found in the first list and never reaches the loop.
// ============================================================
u16 xrServer::amp_holder_of(const u16 id_parent, const u16 id_entity)
{
	CSE_Abstract* claimed = game->get_entity_from_eid(id_parent);
	if (!claimed)
		return id_parent;

	xr_vector<u16>& C = claimed->children;
	if (std::find(C.begin(), C.end(), id_entity) != C.end())
		return id_parent;

	for (xr_vector<u16>::const_iterator it = C.begin(); C.end() != it; ++it)
	{
		CSE_Abstract* box = game->get_entity_from_eid(*it);
		if (!box)
			continue;

		if (std::find(box->children.begin(), box->children.end(), id_entity) != box->children.end())
		{
			Msg("[AMP-S] reject: [%d] is not held by [%d] but by [%d] - detaching there",
			    id_entity, id_parent, *it);
			return *it;
		}
	}

	return id_parent;
}

bool xrServer::Process_event_reject(NET_Packet& P, const ClientID sender, const u32 time, const u16 id_parent,
                                    const u16 id_entity, bool send_message)
{
	// AMP: the claimed holder may only be carrying the real one.
	const u16 holder = amp_holder_of(id_parent, id_entity);

	// Parse message
	CSE_Abstract* e_parent = game->get_entity_from_eid(holder);
	CSE_Abstract* e_entity = game->get_entity_from_eid(id_entity);

	//	R_ASSERT2( e_entity, make_string( "entity not found. parent_id = [%d], entity_id = [%d], frame = [%d]", id_parent, id_entity, Device.dwFrame ).c_str() );
	VERIFY2(e_entity,
	        make_string( "entity not found. parent_id = [%d], entity_id = [%d], frame = [%d]", id_parent, id_entity,
		        Device.dwFrame ).c_str());
	if (!e_entity)
	{
		Msg("! ERROR on rejecting: entity not found. parent_id = [%d], entity_id = [%d], frame = [%d].", id_parent,
		    id_entity, Device.dwFrame);
		return false;
	}

	//	R_ASSERT2( e_parent, make_string( "parent not found. parent_id = [%d], entity_id = [%d], frame = [%d]", id_parent, id_entity, Device.dwFrame ).c_str() );
	VERIFY2(e_parent,
	        make_string( "parent not found. parent_id = [%d], entity_id = [%d], frame = [%d]", id_parent, id_entity,
		        Device.dwFrame ).c_str());
	if (!e_parent)
	{
		Msg("! ERROR on rejecting: parent not found. parent_id = [%d], entity_id = [%d], frame = [%d].", id_parent,
		    id_entity, Device.dwFrame);
		return false;
	}

#ifdef MP_LOGGING
	Msg ( "--- SV: Process reject: parent[%d][%s], item[%d][%s]", id_parent, e_parent->name_replace(), id_entity, e_entity->name());
#endif // MP_LOGGING

	xr_vector<u16>& C = e_parent->children;
	xr_vector<u16>::iterator c = std::find(C.begin(), C.end(), id_entity);
	if (c == C.end())
	{
		xr_string clildrenList;
		for (const u16& childID : e_parent->children)
		{
			clildrenList.append("! ").append(game->get_entity_from_eid(childID)->name_replace()).append("\n");
		}
		Msg("! WARNING: SV: can't find child [%s] of parent [%s]! Children list:\n%s", e_entity->name_replace(),
		    e_parent->name_replace(), clildrenList.c_str());
		return false;
	}

	// AMP: the case is the holder, so say so - to the item, which is what
	// the destroy path reads next, and in the packet everyone else is
	// about to be sent, which still names the man carrying the case.
	// The destination sits at byte 8 of an event packet; byte 6 is the
	// type, as ReplaceOwnershipHeader has always assumed.
	if (holder != id_parent)
	{
		e_entity->ID_Parent = holder;

		if (send_message && P.B.count >= 10)
			CopyMemory(&P.B.data[8], &holder, sizeof(u16));
	}

	if (0xffff == e_entity->ID_Parent)
	{
#ifndef MASTER_GOLD
		Msg	("! ERROR: can't detach independant object. entity[%s][%d], parent[%s][%d], section[%s]",
			e_entity->name_replace(), id_entity, e_parent->name_replace(), holder, e_entity->s_name.c_str() );
#endif // #ifndef MASTER_GOLD
		return (false);
	}

	// Rebuild parentness
	//.	Msg("---ID_Parent [%d], id_parent [%d]", e_entity->ID_Parent, holder);

	//R_ASSERT(e_entity->ID_Parent == holder);
	if (e_entity->ID_Parent != holder)
	{
		Msg("! ERROR: e_entity->ID_Parent = [%d]  parent = [%d][%s]  entity_id = [%d]  frame = [%d]",
		    e_entity->ID_Parent, holder, e_parent->name_replace(), id_entity, Device.dwFrame);
		//it can't be !!!
	}

	game->OnDetach(holder, id_entity);

	//R_ASSERT3(C.end()!=c,e_entity->name_replace(),e_parent->name_replace());
	e_entity->ID_Parent = 0xffff;
	C.erase(c);

	// Signal to everyone (including sender)
	if (send_message)
	{
		DWORD MODE = net_flags(TRUE,TRUE, FALSE, TRUE);
		SendBroadcast(BroadcastCID, P, MODE);
	}

	return (true);
}
