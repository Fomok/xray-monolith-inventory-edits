from pathlib import Path
import argparse,subprocess,json
p=argparse.ArgumentParser();p.add_argument('--compiler',required=True);args=p.parse_args()
E=Path(__file__).resolve().parents[1];B=E.parent/'build';B.mkdir(exist_ok=True)
s=(E/'src/xrGame/script_game_object_inventory_owner.cpp').read_text(encoding='utf-8-sig')
a=s.index('// Native lifecycle for Squared Away');b=s.index('void CScriptGameObject::TakeItem(',a);methods=s[a:b]
# Keep fixture visibility aligned with the real item class. The first fixture
# accidentally made this protected member public and masked MSVC C2248.
import re
item_header=(E/'src/xrGame/inventory_item.h').read_text(encoding='utf-8-sig')
def visibility(symbol):
 prefix=item_header[:item_header.index(symbol)]
 return re.findall(r'\b(public|protected|private)\s*:',prefix)[-1]
assert visibility('SqaTransferGeneration()')=='public'
assert visibility('inventory_rig_transfer::Token sqa_transfer_generation')=='protected'
assert 'SqaTransferGeneration() const { return sqa_transfer_generation; }' in item_header
assert '->sqa_transfer_generation' not in methods
head='#include '+json.dumps((E/'tests/rig_transfer_test_host.h').as_posix())+'\n'
bridge=r'''
extern "C" {
__declspec(dllexport) void host_reset(){live.clear();server_ids.clear();sent_events.clear();warnings=0;Device.dwTimeGlobal=0;}
__declspec(dllexport) void host_add(int id,int kind,int parent){
 CGameObject* o=nullptr;
 if(kind==0)o=new CActor(u16(id));else if(kind==1)o=new CInventoryContainer(u16(id));else if(kind==2)o=new Item(u16(id));else if(kind==4)o=new CInventoryBox(u16(id));else if(kind==5)o=new CAI_Stalker(u16(id));else o=new CGameObject(u16(id));
 o->parent=get(u16(parent));live[u16(id)].reset(o);server_ids.insert(u16(id));
}
__declspec(dllexport) void host_parent(int id,int parent){if(auto* o=get(u16(id)))o->parent=get(u16(parent));}
__declspec(dllexport) void host_server(int id,int present){if(present)server_ids.insert(u16(id));else server_ids.erase(u16(id));}
__declspec(dllexport) void host_destroy(int id){server_ids.erase(u16(id));live.erase(u16(id));}
__declspec(dllexport) void host_clock(unsigned time){Device.dwTimeGlobal=time;}
__declspec(dllexport) int host_request(int who,int id,int rig,int into){
 if(!get(u16(who)))return false;CScriptGameObject self(get(u16(who))),item(get(u16(id))),box(get(u16(rig)));
 return self.SqaRigTransfer(get(u16(id))?&item:nullptr,get(u16(rig))?&box:nullptr,into!=0);
}
__declspec(dllexport) int host_query(int kind,int id){
 CScriptGameObject self(get(1));if(!get(1))return kind==3||kind==4?65535:0;
 switch(kind){case 0:return self.SqaRigTransferCount();case 1:return self.SqaRigTransferPending(u16(id));case 2:return self.SqaRigTransferRigPending(u16(id));case 3:return self.SqaRigTransferAt(id);case 4:return self.SqaRigTransferFinished();case 5:self.SqaRigTransferForget(u16(id));return 0;case 6:dynamic_cast<CActor*>(get(1))->inventory().sqa_rig_transfers.clear();return 0;}return 0;
}
__declspec(dllexport) int host_events(){return int(sent_events.size());}
}
'''
tests=r'''
#include <stdexcept>
#include <cstdio>
#define CHECK(x) do{if(!(x))throw std::runtime_error(#x);}while(0)
void setup(){host_reset();host_add(1,0,65535);host_add(14,1,1);host_add(21,2,14);host_add(22,2,14);}
int main(){try{
 setup();CHECK(host_request(1,21,14,0));CHECK(sent_events.size()==2);CHECK(sent_events[0].type==GE_TRADE_SELL&&sent_events[0].dest==14&&sent_events[0].item==21);CHECK(sent_events[1].type==GE_TRADE_BUY&&sent_events[1].dest==1);CHECK(host_query(1,21)&&host_query(2,14));CHECK(host_query(3,1)==21&&host_query(3,0)==65535);
 CHECK(host_request(1,21,14,0)&&sent_events.size()==2);CHECK(!host_request(1,21,14,1));
 host_parent(21,65535);CHECK(host_query(1,21));host_clock(20000);CHECK(host_query(0,0)==1&&warnings==1);host_clock(30000);CHECK(host_query(0,0)==1&&warnings==1);CHECK(!host_request(1,21,14,1));
 host_parent(21,1);CHECK(!host_query(1,21));CHECK(host_query(4,0)==21&&host_query(4,0)==65535);CHECK(host_request(1,21,14,1)&&sent_events.size()==4);
 host_parent(21,14);CHECK(!host_query(1,21)&&host_query(4,0)==65535);
 setup();host_server(14,0);CHECK(!host_request(1,21,14,0)&&sent_events.empty());
 setup();CHECK(host_request(1,21,14,0));host_server(14,0);CHECK(!host_query(1,21));
 setup();CHECK(host_request(1,21,14,0));host_add(90,3,65535);host_parent(21,90);CHECK(!host_query(1,21)&&host_query(4,0)==65535);CHECK(!host_request(1,21,14,0));
 setup();CHECK(host_request(1,21,14,0));host_add(21,2,1);CHECK(!host_query(1,21)&&host_query(4,0)==65535); // recycled item ID
 setup();CHECK(host_request(1,21,14,0));host_add(14,1,1);CHECK(!host_query(1,21)&&host_query(4,0)==65535); // recycled rig ID
 setup();CHECK(host_request(1,21,14,0));host_query(5,14);CHECK(!host_query(0,0));
 setup();CHECK(host_request(1,21,14,0));host_query(6,0);CHECK(!host_query(0,0)&&host_query(4,0)==65535);
 setup();CHECK(!host_request(14,21,14,0));CHECK(!host_request(1,14,14,1));CHECK(!host_request(1,999,14,0));host_add(90,3,1);CHECK(!host_request(1,90,14,1));host_add(91,1,1);CHECK(!host_request(1,91,14,1));CHECK(sent_events.empty());
 setup();for(int i=0;i<100;++i){CHECK(host_request(1,21,14,0));CHECK(host_request(1,22,14,0));host_parent(21,65535);CHECK(host_query(0,0)==2);host_parent(21,1);CHECK(host_query(0,0)==1);host_parent(22,1);CHECK(host_query(0,0)==0);CHECK(host_query(4,0)!=65535);CHECK(host_query(4,0)!=65535);CHECK(host_query(4,0)==65535);CHECK(host_request(1,21,14,1));CHECK(host_request(1,22,14,1));host_parent(21,14);host_parent(22,14);CHECK(host_query(0,0)==0);CHECK(host_query(4,0)==65535);}CHECK(sent_events.size()==800);
 // Actual container equipment API: reserve before dispatch, preserve old item,
 // reject invalid/pending/destroyed endpoints without sending partial events.
 auto equip=[](int slot,bool back){CScriptGameObject actor(get(1)),item(get(21)),rig(get(14));return actor.SqaEquipFromContainer(&item,&rig,u16(slot),back);};
 setup();CHECK(equip(2,false));CHECK(sent_events.size()==2);CHECK(dynamic_cast<CActor*>(get(1))->inventory().sqa_rig_transfers.entries()[0].target_slot==2);CHECK(!equip(2,false)&&sent_events.size()==2);
 setup();host_add(30,2,1);auto* actor=dynamic_cast<CActor*>(get(1));actor->inventory().slots[2]=dynamic_cast<CInventoryItem*>(get(30));CHECK(equip(2,true));CHECK(sent_events.size()==4);CHECK(sent_events[0].item==30&&sent_events[0].type==GE_TRADE_SELL);CHECK(sent_events[1].item==30&&sent_events[1].dest==14);CHECK(sent_events[2].item==21&&sent_events[3].dest==1);CHECK(host_query(0,0)==2);
 setup();host_add(30,2,1);actor=dynamic_cast<CActor*>(get(1));actor->inventory().slots[2]=dynamic_cast<CInventoryItem*>(get(30));CHECK(equip(2,false));CHECK(sent_events.size()==3&&sent_events[0].type==GEG_PLAYER_ITEM2RUCK);
 setup();CHECK(!equip(65535,false)&&sent_events.empty());CHECK(!equip(99,false)&&sent_events.empty());
 setup();actor=dynamic_cast<CActor*>(get(1));actor->inventory().allow=false;CHECK(!equip(2,false)&&sent_events.empty());
 setup();actor=dynamic_cast<CActor*>(get(1));actor->inventory().take=false;CHECK(!equip(2,false)&&sent_events.empty());
 setup();host_parent(21,1);CHECK(!equip(2,false)&&sent_events.empty());
 setup();host_add(30,2,1);actor=dynamic_cast<CActor*>(get(1));actor->inventory().slots[2]=dynamic_cast<CInventoryItem*>(get(30));host_server(30,0);CHECK(!equip(2,true)&&sent_events.empty()&&host_query(0,0)==0);
 setup();host_add(30,1,1);actor=dynamic_cast<CActor*>(get(1));actor->inventory().slots[2]=dynamic_cast<CInventoryItem*>(get(30));CHECK(!equip(2,true)&&sent_events.empty());
 setup();host_add(14,4,65535);host_parent(21,14);CHECK(equip(1,false));CHECK(sent_events.size()==2);actor=dynamic_cast<CActor*>(get(1));host_parent(21,1);host_arrival(*actor,get(21));CHECK(dynamic_cast<CInventoryItem*>(get(21))->m_ItemCurrPlace.slot_id==1);CHECK(host_query(0,0)==0);
 setup();host_add(14,4,65535);host_parent(21,14);host_add(30,2,1);actor=dynamic_cast<CActor*>(get(1));actor->inventory().slots[1]=dynamic_cast<CInventoryItem*>(get(30));CHECK(equip(1,true));CHECK(sent_events.size()==4&&sent_events[0].item==30&&sent_events[1].dest==14&&sent_events[3].dest==1);CHECK(host_query(0,0)==2);
 setup();host_add(14,4,65535);host_parent(21,14);dynamic_cast<CInventoryBox*>(get(14))->allowed=false;CHECK(!equip(1,false)&&sent_events.empty());
 setup();host_add(14,4,65535);host_parent(21,14);CHECK(equip(1,false));host_add(14,4,65535);actor=dynamic_cast<CActor*>(get(1));host_parent(21,1);host_arrival(*actor,get(21));CHECK(dynamic_cast<CInventoryItem*>(get(21))->m_ItemCurrPlace.type==0&&host_query(0,0)==0);
 auto storage=[](int to){CScriptGameObject self(get(1)),item(get(21)),dest(get(u16(to)));return self.SqaStorageTransfer(&item,&dest);};
 setup();host_add(40,1,1);CHECK(storage(40));CHECK(sent_events.size()==2&&sent_events[0].dest==14&&sent_events[1].dest==40);CHECK(host_query(2,14)&&host_query(2,40));CHECK(storage(40)&&sent_events.size()==2);CHECK(!storage(1));host_parent(21,40);CHECK(host_query(0,0)==0);
 setup();host_add(40,1,1);CHECK(storage(40));host_add(40,1,1);CHECK(host_query(0,0)==0); // replaced destination lifetime
 setup();CHECK(storage(1));host_parent(21,1);actor=dynamic_cast<CActor*>(get(1));host_arrival(*actor,get(21));CHECK(dynamic_cast<CInventoryItem*>(get(21))->m_ItemCurrPlace.type==2);CHECK(host_query(0,0)==0);
 setup();host_add(14,4,65535);host_parent(21,14);CHECK(storage(1));host_parent(21,1);actor=dynamic_cast<CActor*>(get(1));host_arrival(*actor,get(21));CHECK(dynamic_cast<CInventoryItem*>(get(21))->m_ItemCurrPlace.type==2);
 setup();host_add(14,4,65535);host_parent(21,14);host_add(40,1,1);CHECK(storage(40));CHECK(sent_events[1].dest==40);host_parent(21,40);CHECK(host_query(0,0)==0);
 setup();host_add(14,4,65535);host_parent(21,14);dynamic_cast<CInventoryBox*>(get(14))->allowed=false;CHECK(!storage(1)&&sent_events.empty());
 setup();CHECK(!storage(14)&&sent_events.empty());host_add(40,3,65535);CHECK(!storage(40)&&sent_events.empty());

 setup();host_add(14,5,65535);host_parent(21,14);CHECK(equip(2,false));
 CHECK(sent_events.size()==2&&sent_events[0].dest==14&&sent_events[1].dest==1);
 actor=dynamic_cast<CActor*>(get(1));host_parent(21,1);host_arrival(*actor,get(21));
 CHECK(dynamic_cast<CInventoryItem*>(get(21))->m_ItemCurrPlace.type==eItemPlaceSlot);
 CHECK(dynamic_cast<CInventoryItem*>(get(21))->m_ItemCurrPlace.slot_id==2);
 CHECK(host_query(0,0)==0&&host_query(4,0)==21);
 setup();host_add(14,5,65535);host_parent(21,14);dynamic_cast<CAI_Stalker*>(get(14))->alive=true;CHECK(!equip(2,false)&&sent_events.empty());
 setup();host_add(14,5,65535);host_parent(21,14);host_add(30,2,1);actor=dynamic_cast<CActor*>(get(1));actor->inventory().slots[2]=dynamic_cast<CInventoryItem*>(get(30));
 CHECK(equip(2,true));CHECK(sent_events.size()==4&&sent_events[1].dest==14&&sent_events[1].item==30);
 setup();host_add(14,5,65535);host_parent(21,14);CHECK(equip(2,false));host_add(14,5,65535);CHECK(host_query(0,0)==0&&host_query(4,0)==65535);
 setup();host_add(14,5,65535);host_parent(21,14);CHECK(equip(2,false));host_parent(21,65535);CHECK(host_query(1,21));host_parent(21,1);host_add(14,5,65535);
 actor=dynamic_cast<CActor*>(get(1));host_arrival(*actor,get(21));CHECK(dynamic_cast<CInventoryItem*>(get(21))->m_ItemCurrPlace.type==0);
 setup();host_add(14,5,65535);host_parent(21,14);get(14)->dying=true;CHECK(!equip(2,false)&&sent_events.empty());
 puts("PASS: production dispatch/status/completion, async gaps, duplicates, conflicts, destruction, ID reuse, external owners, teardown and 100 handover cycles");return 0;
}catch(const std::exception& e){puts(e.what());return 1;}}
'''
# Compile the real arrival block as well, rather than a duplicate of its logic.
arrival=(E/'src/xrGame/Actor_Events.cpp').read_text(encoding='utf-8-sig')
a=arrival.index('                // Resolve the reserved destination')
b=arrival.index('                inventory().Take(_GO, false, true);',a)
arrival=arrival[a:b].replace('inventory()', 'self.inventory()').replace('transfer.to == ID()', 'transfer.to == self.ID()').replace('transfer.to != ID()', 'transfer.to != self.ID()')
bridge+='\nconstexpr int eItemPlaceSlot=1,eItemPlaceRuck=2;\nvoid host_arrival(CActor& self,CGameObject* _GO){const auto id=_GO->ID();\n'+arrival+'\n}\n'
tests=tests.replace(' puts("PASS: production dispatch/status/completion', ''' setup();CHECK(equip(2,false));actor=dynamic_cast<CActor*>(get(1));host_parent(21,1);host_arrival(*actor,get(21));CHECK(dynamic_cast<CInventoryItem*>(get(21))->m_ItemCurrPlace.type==eItemPlaceSlot);CHECK(dynamic_cast<CInventoryItem*>(get(21))->m_ItemCurrPlace.slot_id==2);
 setup();CHECK(equip(2,false));actor=dynamic_cast<CActor*>(get(1));host_add(30,2,1);actor->inventory().slots[2]=dynamic_cast<CInventoryItem*>(get(30));host_parent(21,1);host_arrival(*actor,get(21));CHECK(dynamic_cast<CInventoryItem*>(get(21))->m_ItemCurrPlace.type==0);
 setup();CHECK(equip(2,false));actor=dynamic_cast<CActor*>(get(1));host_add(21,2,1);host_arrival(*actor,get(21));CHECK(dynamic_cast<CInventoryItem*>(get(21))->m_ItemCurrPlace.type==0);
 setup();CHECK(equip(2,false));actor=dynamic_cast<CActor*>(get(1));host_add(14,1,1);host_parent(21,1);host_arrival(*actor,get(21));CHECK(dynamic_cast<CInventoryItem*>(get(21))->m_ItemCurrPlace.type==0);
 puts("PASS: production dispatch/status/completion''')
cpp=B/'rig_transfer_api_test.cpp';exe=B/'rig_transfer_api_test.exe';cpp.write_text(head+methods+bridge+tests,encoding='utf-8')
subprocess.run([args.compiler,'c++','-std=c++17',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
cpp=B/'rig_transfer_api_bridge.cpp';dll=B/'rig_transfer_api_bridge.dll';cpp.write_text(head+methods+bridge,encoding='utf-8')
subprocess.run([args.compiler,'c++','-std=c++17','-shared',str(cpp),'-o',str(dll)],check=True)
assert 'sqa_rig_transfers.clear();' in (E/'src/xrGame/Inventory.cpp').read_text(encoding='utf-8-sig')
assert 'sqa_transfer_generation = inventory_rig_transfer::next_identity();' in (E/'src/xrGame/inventory_item.cpp').read_text(encoding='utf-8-sig')
print('PASS: actual native API bridge built for Lua handover integration tests')
