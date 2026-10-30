from pathlib import Path
import argparse,subprocess,tempfile
R=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--compiler',required=True);args=p.parse_args()
s=(R/'src/xrGame/Inventory.cpp').read_text(encoding='utf8')
a=s.index('namespace\n{\nstruct SqaSlotRegistry');b=s.index('CInventorySlot::CInventorySlot()',a);registry=s[a:b]
i=(R/'src/xrGame/inventory_item.cpp').read_text(encoding='utf8');a=i.index('\tm_ItemCurrPlace.value = packet.r_u16();',i.index('void CInventoryItem::load'));b=i.index('\tm_fCondition = packet.r_float();',a);load=i[a:b]
h=(R/'src/xrServerEntities/inventory_space.h').read_text();a=h.index('struct SInvItemPlace');b=h.index('\nextern u16',a);place=h[a:b]
head=r'''
#include <cassert>
#include <cstdio>
#include <map>
#include <cstring>
#include <cstdlib>
#include "SqaEquipmentSlots.h"
using u16=unsigned short;using u32=unsigned;using LPCSTR=const char*;using string64=char[64];
#define xr_sprintf std::sprintf
#define Msg(...) ((void)0)
struct Settings {
 std::map<std::string,std::map<std::string,std::string>> data;
 bool section_exist(const char*s){return data.count(s);}
 bool line_exist(const char*s,const char*k){return data[s].count(k);}
 bool r_bool(const char*s,const char*k){return data[s][k]=="true";}
 const char* r_string(const char*s,const char*k){return data[s][k].c_str();}
 bool r_line(const char*s,unsigned n,const char**k,const char**v){if(n>=data[s].size())return false;auto it=data[s].begin();std::advance(it,n);*k=it->first.c_str();*v=it->second.c_str();return true;}
} settings;auto pSettings=&settings;
int SqaConfiguredEquipmentSlot(LPCSTR);u16 SqaEquipmentSlot(LPCSTR);
struct Object {std::string section;const std::string&cNameSect(){return section;}};
struct Item {Object obj;Object& object(){return obj;}};using PIItem=Item*;
'''
head+=registry+'\nenum{eItemPlaceUndefined,eItemPlaceSlot,eItemPlaceBelt,eItemPlaceRuck};\n'+place+r'''
bool read(const char*,bool){return true;}int read(const char*,int){return -1;}
#define READ_IF_EXISTS(settings,method,section,key,fallback) read(key,fallback)
struct Packet {u16 value;u16 r_u16(){return value;}};
struct Saved:Item {SInvItemPlace m_ItemCurrPlace; void load(Packet&packet){
'''+load+'\n}};\n'
main=r'''
int main(int argc,char**argv){
 unsigned legacy=std::atoi(argv[1]);
 for(unsigned n=1;n<=legacy;++n){settings.data["inventory"]["slot_active_"+std::to_string(n)]="true";settings.data["inventory"]["slot_persistent_"+std::to_string(n)]="false";}
 settings.data["sqa_equipment_slots"]={{"hip","true"},{"disabled","false"}};
 settings.data["ampbox_hip"]["sqa_equipment_slot"]="hip";
 settings.data["addon_hip"]["sqa_equipment_slot"]="hip";
 settings.data["unknown"]["sqa_equipment_slot"]="absent";
 unsigned expected=legacy<63?legacy+1:0;
 assert(SqaEquipmentSlot("hip")==expected);assert(!SqaEquipmentSlot("missing"));assert(!SqaEquipmentSlot("disabled"));
 Item hip{{"ampbox_hip"}},addon{{"addon_hip"}},plain{{"plain"}},unknown{{"unknown"}};
 for(unsigned n=1;n<64;++n){assert(SqaSlotAccepts(&hip,n)==(expected&&n==expected));assert(SqaSlotAccepts(&addon,n)==(expected&&n==expected));assert(SqaSlotAccepts(&plain,n)==(n<=legacy));assert(!SqaSlotAccepts(&unknown,n));}
 for(int type:{eItemPlaceSlot,eItemPlaceBelt,eItemPlaceRuck,eItemPlaceUndefined})for(unsigned old:{0u,16u,19u,31u,63u}){
  Saved item;item.obj.section="ampbox_hip";SInvItemPlace saved{};saved.type=type;saved.slot_id=old;saved.base_slot_id=old;Packet packet{saved.value};item.load(packet);
  assert(item.m_ItemCurrPlace.base_slot_id==expected);
  assert(item.m_ItemCurrPlace.type==(type==eItemPlaceSlot&&!expected?eItemPlaceRuck:type));
  assert(item.m_ItemCurrPlace.slot_id==(type==eItemPlaceSlot?expected:old));
  packet.value=item.m_ItemCurrPlace.value;item.load(packet);assert(packet.value==item.m_ItemCurrPlace.value);
 }
 for(unsigned n=0;n<64;++n){auto names=SqaEquipmentSlots::Allocate(n,{"hip","zeta","hip","alpha"});if(n<=60){assert(names.size()==3);assert(SqaEquipmentSlots::Resolve(n,names,"hip")==n+2);}else assert(names.empty());}
 assert(SqaEquipmentSlots::Allocate(64,{"hip"}).empty());
 printf("PASS named slot registry/admission/save round trips legacy=%u hip=%u\n",legacy,expected);
}
'''
# Safety checks verify the production call sites, not a replacement implementation.
assert 'slot_id > SqaSlots().legacyLast) && !CanPutInSlot' in s
assert '!SqaSlotAccepts(pIItem, slot_id)' in s[s.index('bool CInventory::Slot'):]
assert 'i < 16 && (mask & (1u << i))' in s
ui=(R/'src/xrGame/ui/UIActorMenu.h').read_text();assert ui.count('[64]')>=3
assert 'slot_idx < 64 && m_pInvList[slot_idx]' in (R/'src/xrGame/ui/UIActorMenuInventory.cpp').read_text()
with tempfile.TemporaryDirectory(dir=R/'tests') as tmp:
 tmp=Path(tmp);cpp=tmp/'named.cpp';exe=tmp/'named.exe';cpp.write_text(head+main)
 subprocess.run([args.compiler,'-std=c++17','-I',str(R/'src/xrGame'),str(cpp),'-o',str(exe)],check=True)
 for legacy in (13,18,24,31,62,63):subprocess.run([str(exe),str(legacy)],check=True)
print('PASS native admission/UI/mask guard source checks; no game runtime exercised')
