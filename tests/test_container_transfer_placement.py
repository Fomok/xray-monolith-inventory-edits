from pathlib import Path
import subprocess,tempfile,argparse
r=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--compiler',required=True);args=parser.parse_args()
def function(s,name):
 a=s.index(name);b=s.index('{',a);depth=1;e=b+1
 while depth:depth+=(s[e]=='{')-(s[e]=='}');e+=1
 return s[a:e]
event=function((r/'src/xrGame/InventoryContainer.cpp').read_text(encoding='utf-8'),'void CInventoryContainer::OnEvent(')
take=(r/'src/xrGame/inventory.cpp').read_text(encoding='utf-8');a=take.index('\tif (!strict_placement)',take.index('void CInventory::Take('));b=take.index('\n\tm_pOwner->OnItemTake',a);placement=take[a:b]
head=r'''
#include <cassert>
#include <vector>
#include <algorithm>
#include <cstdio>
using u16=unsigned short; using u8=unsigned char;
#define VERIFY(x) assert(x)
#define FALSE false
#define Msg(...) ((void)0)
template<class T> using xr_vector=std::vector<T>;
template<class T,class U>T smart_cast(U* p){return dynamic_cast<T>(p);}
enum {GE_TRADE_BUY=1,GE_OWNERSHIP_TAKE,GE_TRADE_SELL,GE_OWNERSHIP_REJECT};
enum {eItemPlaceUndefined,eItemPlaceBelt,eItemPlaceRuck,eItemPlaceSlot};
struct CObject{virtual ~CObject(){};virtual void H_SetParent(CObject*,bool=false){};void setVisible(bool){};void setEnabled(bool){};};
struct CInventoryItem:CObject{
 struct Place{int type=eItemPlaceUndefined;int slot_id=2;}m_ItemCurrPlace;
 bool cargo=false;void H_SetParent(CObject* p,bool=false)override{if(!p)m_ItemCurrPlace.type=eItemPlaceUndefined;}
 int CurrPlace(){return m_ItemCurrPlace.type;}int BaseSlot(){return 2;}bool RuckDefault(){return cargo;}
};
CInventoryItem* current=nullptr;
struct World{struct ObjectsT{CObject* net_Find(u16){return current;}}Objects;}world;
World& Level(){return world;}
struct NET_Packet{void r_u16(u16& id){id=21;}bool r_eof(){return true;}u8 r_u8(){return 0;}};
struct Base:CInventoryItem{void OnEvent(NET_Packet&,u16){}};
struct CInventoryContainer:Base{
 using inherited=Base;std::vector<u16>m_items;int ID(){return 14;}void RecalcOwnerWeight(){};void OnEvent(NET_Packet&,u16);
};
struct CInventory{
 bool free_slot=true,free_belt=false;int slots=0,rucks=0,belts=0;
 bool CanPutInSlot(CInventoryItem*,int){return free_slot;}
 bool CanPutInBelt(CInventoryItem*){return free_belt;}
 bool Slot(int,CInventoryItem* i,bool,bool){i->m_ItemCurrPlace.type=eItemPlaceSlot;++slots;return true;}
 bool Belt(CInventoryItem* i,bool){i->m_ItemCurrPlace.type=eItemPlaceBelt;++belts;return true;}
 bool Ruck(CInventoryItem* i,bool){i->m_ItemCurrPlace.type=eItemPlaceRuck;++rucks;return true;}
 void take(CInventoryItem* pIItem,bool strict_placement){bool bNotActivate=false;PLACEMENT}
};
'''.replace('PLACEMENT',placement)
main=r'''
int main(){
 for(bool slot:{false,true})for(bool belt:{false,true})for(bool cargo:{false,true}){
  CInventoryItem item;item.cargo=cargo;item.m_ItemCurrPlace.type=eItemPlaceSlot;current=&item;
  CInventoryContainer rig;rig.m_items={21};NET_Packet p;rig.OnEvent(p,GE_TRADE_SELL);
  assert(rig.m_items.empty());CInventory actor;actor.free_slot=slot;actor.free_belt=belt;actor.take(&item,true);
  assert(actor.rucks==1&&actor.slots==0&&actor.belts==0);
 }
 // A deliberate world drop must retain ordinary pickup behaviour.
 CInventoryItem pistol;current=&pistol;CInventoryContainer rig;rig.m_items={21};NET_Packet p;
 rig.OnEvent(p,GE_OWNERSHIP_REJECT);CInventory actor;actor.take(&pistol,true);assert(actor.slots==1);
 // NPCs that request non-strict placement retain their original selection.
 rig.m_items={21};rig.OnEvent(p,GE_TRADE_SELL);CInventory npc;npc.take(&pistol,false);assert(npc.slots==1);
 puts("PASS: 8 container-transfer combinations, world drop and NPC placement");
}
'''
# The old event handler must fail the empty-slot pistol case.
start=event.index('\n\t\t\t// Contents transferred out');end=event.index('\n\t\t\tRecalcOwnerWeight();',start)
old=event[:start]+event[end:]
with tempfile.TemporaryDirectory() as tmp:
 tmp=Path(tmp)
 for label,body,success in [('baseline',old,False),('fixed',event,True)]:
  cpp=tmp/(label+'.cpp');exe=tmp/(label+'.exe');cpp.write_text(head+body+main)
  built=subprocess.run([args.compiler,'c++','-std=c++17',str(cpp),'-o',str(exe)],capture_output=True,text=True)
  assert built.returncode==0,built.stderr
  ran=subprocess.run([str(exe)],capture_output=True,text=True)
  assert (ran.returncode==0)==success,(label,ran.stdout,ran.stderr)
  print('PASS: original auto-equip reproduced' if not success else ran.stdout.strip())

