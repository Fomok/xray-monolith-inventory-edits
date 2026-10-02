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
 if(kind==0)o=new CActor(u16(id));else if(kind==1)o=new CInventoryContainer(u16(id));else if(kind==2)o=new Item(u16(id));else o=new CGameObject(u16(id));
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
 puts("PASS: production dispatch/status/completion, async gaps, duplicates, conflicts, destruction, ID reuse, external owners, teardown and 100 handover cycles");return 0;
}catch(const std::exception& e){puts(e.what());return 1;}}
'''
cpp=B/'rig_transfer_api_test.cpp';exe=B/'rig_transfer_api_test.exe';cpp.write_text(head+methods+bridge+tests,encoding='utf-8')
subprocess.run([args.compiler,'c++','-std=c++17',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
cpp=B/'rig_transfer_api_bridge.cpp';dll=B/'rig_transfer_api_bridge.dll';cpp.write_text(head+methods+bridge,encoding='utf-8')
subprocess.run([args.compiler,'c++','-std=c++17','-shared',str(cpp),'-o',str(dll)],check=True)
assert 'sqa_rig_transfers.clear();' in (E/'src/xrGame/Inventory.cpp').read_text(encoding='utf-8-sig')
assert 'sqa_transfer_generation = inventory_rig_transfer::next_identity();' in (E/'src/xrGame/inventory_item.cpp').read_text(encoding='utf-8-sig')
print('PASS: actual native API bridge built for Lua handover integration tests')
