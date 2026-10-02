from pathlib import Path
import subprocess,sys
R=Path(__file__).resolve().parents[2]; E=R/'engine'
s=(E/'src/xrGame/alife_dynamic_object.cpp').read_text(encoding='utf-8-sig')
def extract(name):
 a=s.index('void '+name+'::add_online');b=s.index('\n}',a)+2
 return s[a:b]
head=r'''
#include <vector>
#include <map>
#include <cassert>
#include <cstdio>
using u16=unsigned short;using u32=unsigned;const bool FALSE=false;const unsigned M_SPAWN_UPDATE=1;
namespace ALife {using OBJECT_VECTOR=std::vector<u16>;using OBJECT_IT=OBJECT_VECTOR::iterator;}
struct NET_Packet{};struct ClientID{void set(unsigned){} unsigned value(){return 0;}};
struct Flags {void or(unsigned){}void and(unsigned){}};
struct CSE_Abstract {virtual ~CSE_Abstract()=default; Flags s_flags;};
struct World;
struct CSE_ALifeDynamicObject:CSE_Abstract {
 u16 ID=0;int o_Position=0,m_tNodeID=0;bool m_bOnline=false;ALife::OBJECT_VECTOR children;
 World& alife();virtual void add_online(const bool&){assert(children.empty());}
};
struct CSE_ALifeDynamicObjectVisual:CSE_ALifeDynamicObject {void add_online(const bool&) override {}};
struct CSE_ALifeInventoryBox:CSE_ALifeDynamicObjectVisual {void add_online(const bool&) override;};
struct CSE_ALifeInventoryItem:CSE_ALifeDynamicObject {CSE_Abstract* base(){return this;}};
struct CSE_ALifeItem:CSE_ALifeInventoryItem {void add_online(const bool&) override{}};
struct CSE_ALifeItemContainer:CSE_ALifeItem {void add_online(const bool&) override;};
struct Server {
 struct Client {ClientID ID;};Client* GetServerClient(){return nullptr;}
 std::vector<u16> spawned;
 void entity_Destroy(CSE_Abstract*){}
 void Process_spawn(NET_Packet&,ClientID,bool,CSE_Abstract* item){spawned.push_back(dynamic_cast<CSE_ALifeDynamicObject*>(item)->ID);}
};
struct Objects {std::map<u16,CSE_ALifeDynamicObject*> all;CSE_ALifeDynamicObject* object(u16 id,bool optional=false){auto p=all[id];assert(optional||p);return p;}};
struct World {Server srv;Objects objs;Server& server(){return srv;}Objects& objects(){return objs;}} world;
struct AI {World& alife(){return world;}} ai_instance;
AI& ai(){return ai_instance;}World& CSE_ALifeDynamicObject::alife(){return world;}
template<class T,class U>T smart_cast(U* p){return dynamic_cast<T>(p);}
#define R_ASSERT2(v,msg) assert(v)
#define Msg(...) ((void)0)
'''
main=r'''
int main(){
 CSE_ALifeInventoryBox stash;stash.ID=1;stash.o_Position=71;stash.m_tNodeID=82;
 CSE_ALifeItemContainer rig,box,nested;CSE_ALifeInventoryItem med,mag,ammo,plain;
 rig.ID=2;box.ID=3;nested.ID=4;med.ID=5;mag.ID=6;ammo.ID=7;plain.ID=8;
 stash.children={2,3,8};rig.children={5,6};box.children={4};nested.children={7};
 for(CSE_ALifeDynamicObject* p:std::vector<CSE_ALifeDynamicObject*>{&stash,&rig,&box,&nested,&med,&mag,&ammo,&plain})world.objs.all[p->ID]=p;
 for(int cycle=0;cycle<3;++cycle){
  world.srv.spawned.clear();for(auto p:world.objs.all)p.second->m_bOnline=false;
  stash.add_online(true);
  assert((world.srv.spawned==std::vector<u16>{2,5,6,3,4,7,8}));
  for(u16 id:{2,3,4,5,6,7,8}){auto p=world.objs.object(id);assert(p->m_bOnline&&p->o_Position==71&&p->m_tNodeID==82);}
  assert((rig.children==std::vector<u16>{5,6})&&(nested.children==std::vector<u16>{7}));
 }
 stash.children.clear();world.srv.spawned.clear();stash.add_online(true);assert(world.srv.spawned.empty());
 puts("PASS: production stash and container online methods restore rigs, boxes, nested contents and ordinary items across repeated online transitions");
}
'''
p=R/'build/stash_contents_test.cpp';p.write_text(head+extract('CSE_ALifeInventoryBox')+'\n'+extract('CSE_ALifeItemContainer')+main)
exe=R/'build/stash_contents_test.exe'
subprocess.run([str(R/'test-deps/ziglang/zig.exe'),'c++','-std=c++17','-fno-operator-names',str(p),'-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
