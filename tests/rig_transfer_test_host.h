#pragma once
#include <map>
#include <memory>
#include <set>
#include <vector>
#include <cstdint>
#include "../src/xrGame/inventory_rig_transfer.h"
using u16=std::uint16_t;using u32=std::uint32_t;
constexpr int GE_TRADE_SELL=1,GE_TRADE_BUY=2;
struct NET_Packet{int type=0;u16 dest=0,item=0;void w_u16(u16 v){item=v;}};
inline std::vector<NET_Packet> sent_events;
inline unsigned warnings=0;
inline void Msg(const char*,...){++warnings;}
struct CObject{u16 id;CObject* parent=nullptr;bool dying=false;virtual ~CObject()=default;explicit CObject(u16 n):id(n){} u16 ID()const{return id;}CObject* H_Parent(){return parent;}bool getDestroy(){return dying;}};
struct CGameObject:CObject{using CObject::CObject;static void u_EventGen(NET_Packet& p,int type,u16 dest){p={type,dest,0};}static void u_EventSend(NET_Packet& p){sent_events.push_back(p);}};
struct CInventoryItem{
 virtual ~CInventoryItem()=default;
 inventory_rig_transfer::Token SqaTransferGeneration() const { return sqa_transfer_generation; }
protected:
 inventory_rig_transfer::Token sqa_transfer_generation=inventory_rig_transfer::next_identity();
};
struct Item:CGameObject,CInventoryItem{using CGameObject::CGameObject;};
struct CInventoryContainer:Item{using Item::Item;};
struct Inventory{inventory_rig_transfer::Registry sqa_rig_transfers;};
struct CActor:CGameObject{using CGameObject::CGameObject;Inventory bag;Inventory& inventory(){return bag;}};
template<class T,class U>T smart_cast(U* p){return dynamic_cast<T>(p);}
inline std::map<u16,std::unique_ptr<CGameObject>> live;
inline std::set<u16> server_ids;
struct Objects{CObject* net_Find(u16 id){auto it=live.find(id);return it==live.end()?nullptr:it->second.get();}};
struct World{Objects Objects;};
inline World world;
inline World& Level(){return world;}
struct ServerObjects{void* object(u16 id,bool){return server_ids.count(id)?this:nullptr;}};
struct ALife{ServerObjects registry;ServerObjects& objects(){return registry;}};
struct AI{ALife life;ALife* get_alife(){return &life;}ALife& alife(){return life;}};
inline AI ai_instance;
inline AI& ai(){return ai_instance;}
inline struct Clock{u32 dwTimeGlobal=0;}Device;
struct CScriptGameObject{
 CGameObject* ptr;explicit CScriptGameObject(CGameObject* o):ptr(o){} CGameObject& object(){return *ptr;}
 bool SqaRigTransfer(CScriptGameObject*,CScriptGameObject*,bool);
 bool SqaRigTransferPending(u16);bool SqaRigTransferRigPending(u16);
 u32 SqaRigTransferCount();u16 SqaRigTransferAt(u32);u16 SqaRigTransferFinished();void SqaRigTransferForget(u16);
};
inline CGameObject* get(u16 id){auto it=live.find(id);return it==live.end()?nullptr:it->second.get();}
