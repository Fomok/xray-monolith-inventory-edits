#include "stdafx.h"
#include "xrserver.h"
#include "InventoryNextProbe.h"
#include "InventoryNextIdentity.h"
#include "actor.h"
#include "xrserver_objects.h"
#include "xrserver_objects_alife_monsters.h"
#include "xrServer_svclient_validation.h"

void ReplaceOwnershipHeader(NET_Packet& P)
{
	//способ очень грубый, но на данный момент иного выбора нет. Заранее приношу извинения
	u16 NewType = GE_OWNERSHIP_TAKE;
	CopyMemory(&P.B.data[6], &NewType, 2);
};

void xrServer::Process_event_ownership(NET_Packet& P, ClientID sender, u32 time, u16 ID, BOOL bForced, bool sqa_ground)
{
	u32 MODE = net_flags(TRUE,TRUE, FALSE, TRUE);

	u16 id_parent = ID, id_entity;
	P.r_u16(id_entity);
	CSE_Abstract* e_parent = game->get_entity_from_eid(id_parent);
	CSE_Abstract* e_entity = game->get_entity_from_eid(id_entity);


#ifdef MP_LOGGING
	Msg( "--- SV: Process ownership take: parent [%d][%s], item [%d][%s]", 
		id_parent, e_parent ? e_parent->name_replace() : "null_parent",
		id_entity, e_entity ? e_entity->name() : "null_entity");
#endif // MP_LOGGING

	if (!e_parent)
	{
		Msg("! ERROR on ownership: parent not found. parent_id = [%d], entity_id = [%d], frame = [%d].", id_parent,
		    id_entity, Device.dwFrame);
		return;
	}
	if (!e_entity)
	{
		//		Msg( "! ERROR on ownership: entity not found. parent_id = [%d], entity_id = [%d], frame = [%d].", id_parent, id_entity, Device.dwFrame );
		return;
	}

	if (!is_object_valid_on_svclient(id_parent))
	{
		Msg(
			"! ERROR on ownership: parent object is not valid on sv client. parent_id = [%d], entity_id = [%d], frame = [%d]",
			id_parent, id_entity, Device.dwFrame);
		return;
	}

	if (!is_object_valid_on_svclient(id_entity))
	{
		Msg(
			"! ERROR on ownership: entity object is not valid on sv client. parent_id = [%d], entity_id = [%d], frame = [%d]",
			id_parent, id_entity, Device.dwFrame);
		return;
	}

	if (0xffff != e_entity->ID_Parent) return;

	xrClientData* c_parent = e_parent->owner;
	xrClientData* c_entity = e_entity->owner;
	xrClientData* c_from = ID_to_client(sender);

	if ((GetServerClient() != c_from) && (c_parent != c_from))
	{
		// trust only ServerClient or new_ownerClient
		return;
	}

	CSE_ALifeCreatureAbstract* alife_entity = smart_cast<CSE_ALifeCreatureAbstract*>(e_parent);
	if (alife_entity && !alife_entity->g_Alive() && game->Type() != eGameIDSingle)
	{
#ifdef MP_LOGGING
		Msg("--- SV: WARNING: dead player [%d] tries to take item [%d]", id_parent, id_entity);
#endif //#ifdef MP_LOGGING
		return;
	};

	if(sqa_ground&&!InventoryNextPickupPrepare(id_parent,id_entity))return;
	// Game allows ownership of entity
	if (game->OnTouch(id_parent, id_entity, bForced))
	{
		// Perform migration if needed
		if (c_parent != c_entity) PerformMigration(e_entity, c_entity, c_parent);

		// Rebuild parentness
		e_entity->ID_Parent = id_parent;
		e_parent->children.push_back(id_entity);
		if(sqa_ground)InventoryNextPickupFinish(id_parent,id_entity,true);

		if (bForced)
		{
			ReplaceOwnershipHeader(P);
		}
		// Signal to everyone (including sender)
		SendBroadcast(BroadcastCID, P, MODE);
	}
	else if(sqa_ground)InventoryNextPickupFinish(id_parent,id_entity,false);
}


// One server event admits before detach. Rejection never emits a client detach.
void xrServer::Process_event_sqa_acquire(NET_Packet& P,ClientID sender,u32 time,u16 owner)
{
    if(!InventoryNextStrictOwnedPickup()||game->Type()!=eGameIDSingle||!Actor()||owner!=Actor()->ID())return;
    const u16 source=P.r_u16(),live=P.r_u16();
    u64 expected_source,expected_item;P.r_u64(expected_source);P.r_u64(expected_item);
    const u16 slot=P.r_u16();
    auto from=game->get_entity_from_eid(source);auto to=game->get_entity_from_eid(owner);auto item=game->get_entity_from_eid(live);
    if(!from||!to||!item||source==owner||item->ID_Parent!=source||
       !is_object_valid_on_svclient(source)||!is_object_valid_on_svclient(owner)||!is_object_valid_on_svclient(live)) {
        InventoryNextPickupStatus("Source unavailable. Transfer cancelled.");return;
    }
    auto client=ID_to_client(sender);
    if(client!=GetServerClient()&&client!=to->owner)return;
    if(!expected_source||!expected_item||InventoryNextIdentity(source)!=expected_source||InventoryNextIdentity(live)!=expected_item) {
        InventoryNextPickupStatus("Source changed. Transfer cancelled.");return;
    }
    if(!InventoryNextPickupPrepare(owner,live,source,slot))return;
    NET_Packet reject;reject.w_begin(M_EVENT);reject.w_u32(time);reject.w_u16(GE_TRADE_SELL);reject.w_u16(source);reject.w_u16(live);
    if(!Process_event_reject(reject,sender,time,source,live,false)) {
        InventoryNextPickupFinish(owner,live,false);return;
    }
    if(!game->OnTouch(owner,live,FALSE)) {
        // Single-player OnTouch normally always accepts. Restore the original
        // ALife/native owner before acknowledging a rejected transaction.
        if(game->OnTouch(source,live,FALSE)){item->ID_Parent=source;from->children.push_back(live);}
        InventoryNextPickupFinish(owner,live,false);return;
    }
    if(from->owner!=to->owner)PerformMigration(item,from->owner,to->owner);
    item->ID_Parent=owner;to->children.push_back(live);
    InventoryNextPickupFinish(owner,live,true);
    NET_Packet take;take.w_begin(M_EVENT);take.w_u32(time);take.w_u16(GE_TRADE_BUY);take.w_u16(owner);take.w_u16(live);
    const u32 mode=net_flags(TRUE,TRUE,FALSE,TRUE);
    SendBroadcast(BroadcastCID,reject,mode);SendBroadcast(BroadcastCID,take,mode);
}

void xrServer::Process_event_sqa_purchase(NET_Packet& P,ClientID sender,u32 time,u16 owner)
{
    if(!InventoryNextStrictTradePickup()||game->Type()!=eGameIDSingle||!Actor()||owner!=Actor()->ID())return;
    const u16 source=P.r_u16();u64 token;P.r_u64(token);
    auto from=game->get_entity_from_eid(source);auto to=game->get_entity_from_eid(owner);
    auto client=ID_to_client(sender);
    if(!to||(client!=GetServerClient()&&client!=to->owner))return;
    xr_vector<u16> items;
    if(!InventoryNextPurchaseValidate(source,owner,token,items))return;
    if(!from){InventoryNextPurchaseFinish(false);return;}
    xr_vector<u16> moved;
    bool accepted=true;
    for(u16 live:items) {
        auto item=game->get_entity_from_eid(live);
        if(!item||item->ID_Parent!=source||!is_object_valid_on_svclient(live)) {accepted=false;break;}
        NET_Packet reject;reject.w_begin(M_EVENT);reject.w_u32(time);reject.w_u16(GE_TRADE_SELL);reject.w_u16(source);reject.w_u16(live);
        if(!Process_event_reject(reject,sender,time,source,live,false)){accepted=false;break;}
        if(!game->OnTouch(owner,live,FALSE)) {
            if(game->OnTouch(source,live,FALSE)){item->ID_Parent=source;from->children.push_back(live);}
            accepted=false;break;
        }
        item->ID_Parent=owner;to->children.push_back(live);moved.push_back(live);
    }
    if(!accepted) {
        // No client detach or payment was emitted. Restore every earlier member
        // through the same ALife hooks before acknowledging rejection.
        for(auto it=moved.rbegin();it!=moved.rend();++it) {
            auto item=game->get_entity_from_eid(*it);if(!item)continue;
            NET_Packet rollback;rollback.w_begin(M_EVENT);rollback.w_u32(time);rollback.w_u16(GE_TRADE_SELL);rollback.w_u16(owner);rollback.w_u16(*it);
            if(Process_event_reject(rollback,sender,time,owner,*it,false)&&game->OnTouch(source,*it,FALSE)) {
                item->ID_Parent=source;from->children.push_back(*it);
            }
        }
        InventoryNextPurchaseFinish(false);return;
    }
    for(u16 live:items) {
        auto item=game->get_entity_from_eid(live);
        if(from->owner!=to->owner)PerformMigration(item,from->owner,to->owner);
    }
    if(!InventoryNextPurchaseFinish(true))return;
    const u32 mode=net_flags(TRUE,TRUE,FALSE,TRUE);
    for(u16 live:items) {
        NET_Packet reject;reject.w_begin(M_EVENT);reject.w_u32(time);reject.w_u16(GE_TRADE_SELL);reject.w_u16(source);reject.w_u16(live);
        NET_Packet take;take.w_begin(M_EVENT);take.w_u32(time);take.w_u16(GE_TRADE_BUY);take.w_u16(owner);take.w_u16(live);
        SendBroadcast(BroadcastCID,reject,mode);SendBroadcast(BroadcastCID,take,mode);
    }
}


void xrServer::Process_event_sqa_world_transfer(NET_Packet& P,ClientID sender,u32 time,u16 actor)
{
    if(!InventoryNextWorldStorageEnabled()||game->Type()!=eGameIDSingle||!Actor()||actor!=Actor()->ID())return;
    const u16 source=P.r_u16(),destination=P.r_u16();u64 token;P.r_u64(token);
    auto from=game->get_entity_from_eid(source);auto to=game->get_entity_from_eid(destination);
    auto player=game->get_entity_from_eid(actor);auto client=ID_to_client(sender);
    if(!player||(client!=GetServerClient()&&client!=player->owner))return;
    xr_vector<u16> items;
    if(!InventoryNextWorldValidate(source,destination,token,items))return;
    if(!from||!to){InventoryNextWorldFinish(false);return;}
    xr_vector<u16> moved;bool accepted=true;
    for(u16 live:items) {
        auto item=game->get_entity_from_eid(live);
        if(!item||item->ID_Parent!=source||!is_object_valid_on_svclient(live)){accepted=false;break;}
        NET_Packet reject;reject.w_begin(M_EVENT);reject.w_u32(time);reject.w_u16(GE_TRADE_SELL);reject.w_u16(source);reject.w_u16(live);
        if(!Process_event_reject(reject,sender,time,source,live,false)){accepted=false;break;}
        if(!game->OnTouch(destination,live,FALSE)) {
            if(game->OnTouch(source,live,FALSE)){item->ID_Parent=source;from->children.push_back(live);}
            accepted=false;break;
        }
        item->ID_Parent=destination;to->children.push_back(live);moved.push_back(live);
        // Isolated, explicitly requested fixture: exercise real ALife rollback
        // after an actual member moved, before any client notification.
        if(moved.size()==1&&strstr(Core.Params,"-sqa3-fixture-world-fail-after-first")) {
            Msg("* SQA3 loaded transfer fixture interrupted after moved=1 before client notification");
            accepted=false;break;
        }
    }
    if(!accepted) {
        for(auto it=moved.rbegin();it!=moved.rend();++it) {
            auto item=game->get_entity_from_eid(*it);if(!item)continue;
            NET_Packet rollback;rollback.w_begin(M_EVENT);rollback.w_u32(time);rollback.w_u16(GE_TRADE_SELL);rollback.w_u16(destination);rollback.w_u16(*it);
            if(Process_event_reject(rollback,sender,time,destination,*it,false)&&game->OnTouch(source,*it,FALSE)) {
                item->ID_Parent=source;from->children.push_back(*it);
            }
        }
        InventoryNextWorldFinish(false);return;
    }
    for(u16 live:items) {
        auto item=game->get_entity_from_eid(live);
        if(from->owner!=to->owner)PerformMigration(item,from->owner,to->owner);
    }
    if(!InventoryNextWorldFinish(true))return;
    const u32 mode=net_flags(TRUE,TRUE,FALSE,TRUE);
    for(u16 live:items) {
        NET_Packet reject;reject.w_begin(M_EVENT);reject.w_u32(time);reject.w_u16(GE_TRADE_SELL);reject.w_u16(source);reject.w_u16(live);
        NET_Packet take;take.w_begin(M_EVENT);take.w_u32(time);take.w_u16(GE_TRADE_BUY);take.w_u16(destination);take.w_u16(live);
        SendBroadcast(BroadcastCID,reject,mode);SendBroadcast(BroadcastCID,take,mode);
    }
}
