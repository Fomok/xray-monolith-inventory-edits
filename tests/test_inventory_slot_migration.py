from pathlib import Path
import argparse,subprocess,tempfile
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--compiler',required=True);args=p.parse_args()
s=(R/'src/xrGame/inventory_item.cpp').read_text(encoding='utf-8');a=s.index('\tm_ItemCurrPlace.value = packet.r_u16();',s.index('void CInventoryItem::load'));b=s.index('\tm_fCondition = packet.r_float();',a);load=s[a:b]
h=(R/'src/xrServerEntities/inventory_space.h').read_text();a=h.index('struct SInvItemPlace');b=h.index('\nextern u16',a);place=h[a:b]
head=r'''
#include <cassert>
#include <cstdio>
#include <cstring>
using u16=unsigned short;
#define Msg(...) ((void)0)
enum{eItemPlaceUndefined,eItemPlaceSlot,eItemPlaceBelt,eItemPlaceRuck};
PLACE
struct Settings {bool enabled=true;int slot=15;} settings;
bool read(const char*,bool fallback){return settings.enabled;}
int read(const char*,int fallback){return settings.slot;}
#define READ_IF_EXISTS(settings,method,section,key,fallback) read(key,fallback)
struct Packet {u16 value;u16 r_u16(){return value;}};
struct Item {SInvItemPlace m_ItemCurrPlace;void load(Packet& packet){LOAD}};
'''.replace('PLACE',place).replace('LOAD',load)
main=r'''
int main(){
 int cases=0;
 for(int type:{eItemPlaceSlot,eItemPlaceBelt,eItemPlaceRuck,eItemPlaceUndefined}){
  Item item{};SInvItemPlace saved{};saved.type=type;saved.slot_id=14;saved.base_slot_id=14;
  Packet p{saved.value};item.load(p);
  assert(item.m_ItemCurrPlace.base_slot_id==16);
  assert(item.m_ItemCurrPlace.type==type);
  assert(item.m_ItemCurrPlace.slot_id==(type==eItemPlaceSlot?16:14));++cases;
  // A second round trip must preserve slot and type.
  p.value=item.m_ItemCurrPlace.value;item.load(p);assert(item.m_ItemCurrPlace.value==p.value);++cases;
 }
 // Opt-out: normal script animation and PDA objects retain their saved slot.
 settings.enabled=false;Item ordinary{};SInvItemPlace saved{};
 saved.type=eItemPlaceSlot;saved.slot_id=14;saved.base_slot_id=14;Packet p{saved.value};
 ordinary.load(p);assert(ordinary.m_ItemCurrPlace.value==p.value);++cases;
 // Opted-in equipment also migrates when its saved base disagrees with placement.
 settings.enabled=true;saved.slot_id=3;p.value=saved.value;ordinary.load(p);
 assert(ordinary.m_ItemCurrPlace.slot_id==16&&ordinary.m_ItemCurrPlace.base_slot_id==16);++cases;
 for(int base:{0,7,14,16}){saved.base_slot_id=base;saved.slot_id=14;p.value=saved.value;ordinary.load(p);
  assert(ordinary.m_ItemCurrPlace.slot_id==16&&ordinary.m_ItemCurrPlace.base_slot_id==16);++cases;}
 // Invalid configuration must not truncate into the 6-bit slot field.
 for(int slot:{-1,63,64}){settings.slot=slot;ordinary.load(p);assert(ordinary.m_ItemCurrPlace.value==p.value);++cases;}
 printf("PASS: %d migration, round-trip, opt-out and invalid-config cases\n",cases);
}
'''
head='#include <initializer_list>\n'+head
with tempfile.TemporaryDirectory() as tmp:
 tmp=Path(tmp)
 for label,code,success in [('baseline',head.replace(load,'m_ItemCurrPlace.value=packet.r_u16();'),False),('preview9',head.replace('if (m_ItemCurrPlace.type == eItemPlaceSlot)', 'if (m_ItemCurrPlace.type == eItemPlaceSlot && m_ItemCurrPlace.slot_id == m_ItemCurrPlace.base_slot_id)'),False),('fixed',head,True)]:
  cpp=tmp/(label+'.cpp');exe=tmp/(label+'.exe');cpp.write_text(code+main)
  build=subprocess.run([args.compiler,'c++','-std=c++17',str(cpp),'-o',str(exe)],capture_output=True,text=True);assert build.returncode==0,build.stderr
  run=subprocess.run([str(exe)],capture_output=True,text=True);assert (run.returncode==0)==success,(label,run.stderr)
  print(run.stdout.strip() if success else 'PASS: '+label+' migration failure reproduced')
