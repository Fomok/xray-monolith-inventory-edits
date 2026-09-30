from pathlib import Path
import subprocess
import argparse
import tempfile
import os
parser=argparse.ArgumentParser(description='Compile extracted engine functions against a small ownership test host.')
parser.add_argument('--compiler',default=os.environ.get('CXX','clang++'))
args=parser.parse_args()
engine=Path(__file__).resolve().parents[1]
workspace=tempfile.TemporaryDirectory(prefix='squared-away-access-')
def function(path, signature):
    s=(engine/path).read_text(encoding='utf-8-sig')
    a=s.index(signature); b=s.index('{',a); depth=1; i=b+1
    while depth:
        if s[i]=='{': depth+=1
        elif s[i]=='}': depth-=1
        i+=1
    return s[a:i]
source=r"""
#include <algorithm>
#include <functional>
#include <map>
#include <vector>
#include <cassert>
#include <cstdio>
using u16=unsigned short;
template<class T> using xr_vector=std::vector<T>;
struct CObject { u16 id; CObject* parent=nullptr; virtual ~CObject()=default; u16 ID()const{return id;} CObject* H_Parent()const{return parent;} };
struct CGameObject: CObject { CGameObject* lua_game_object(){return this;} };
struct CInventoryItem {virtual ~CInventoryItem()=default; virtual CGameObject& object()const=0;};
using PIItem=CInventoryItem*; using TIItemContainer=std::vector<PIItem>;
struct Item: CGameObject,CInventoryItem {CGameObject& object()const override{return *const_cast<Item*>(this);}};
struct CInventoryContainer: Item {std::vector<u16> m_items;};
template<class T,class U> T smart_cast(U* p){return dynamic_cast<T>(p);}
struct Objects {std::map<u16,CObject*> live; CObject* net_Find(u16 id){auto i=live.find(id);return i==live.end()?nullptr:i->second;}};
struct LevelHost {struct Objects Objects;}; LevelHost host; LevelHost& Level(){return host;}
namespace luabind {using object=CGameObject*; template<class T> using functor=std::function<T(void*,void*)>;}
namespace ScriptStorage {enum {eLuaMessageTypeError};}
struct Log {void script_log(int,const char*){}}; struct AI {Log log; Log& script_engine(){return log;}}; AI ai_host; AI& ai(){return ai_host;}
struct CInventoryOwner;
struct CInventory {CInventoryOwner* m_pOwner; TIItemContainer m_all,available; bool AmpInCarriedBox(const CInventoryItem*)const; void AddAvailableItems(TIItemContainer& out,bool){out=available;}};
struct CInventoryOwner: CGameObject {CInventory inv; CInventoryOwner(){inv.m_pOwner=this;} CInventory& inventory(){return inv;} CGameObject* cast_game_object(){return this;}};
struct CScriptGameObject {CGameObject* go; CGameObject& object(){return *go;} void IterateInventory(luabind::functor<bool>,luabind::object);void IterateInventoryDirect(luabind::functor<bool>,luabind::object);void ForEachInventoryItems(const luabind::functor<bool>&);void IterateContainer(luabind::functor<bool>,luabind::object);};
"""
source+=function(Path('src/xrGame/Inventory.cpp'),'bool CInventory::AmpInCarriedBox(')+'\n'
for name in ['IterateInventory','IterateInventoryDirect','ForEachInventoryItems','IterateContainer']:
    source+=function(Path('src/xrGame/script_game_object_inventory_owner.cpp'),'void CScriptGameObject::'+name+'(')+'\n'
source+=r"""
int main(){
 CInventoryOwner actor; actor.id=1; CScriptGameObject api{&actor};
 Item loose, child, stale; CInventoryContainer box;
 loose.id=2; box.id=3; child.id=4; stale.id=5;
 auto reset=[&]{host.Objects.live={{1,&actor},{2,&loose},{3,&box},{4,&child},{5,&stale}};loose.parent=&actor;box.parent=&actor;child.parent=&box;stale.parent=nullptr;box.m_items={4,5};actor.inv.m_all={&loose,&box};actor.inv.available={&loose,&box,&child,&stale};};
 std::vector<u16> got;
 auto collect=[&](void*,void* p){got.push_back(static_cast<CGameObject*>(p)->ID());return false;};
 reset();api.IterateInventory(collect,nullptr);assert((got==std::vector<u16>{2,3,4}));
 assert(actor.inv.AmpInCarriedBox(&child));assert(!actor.inv.AmpInCarriedBox(&stale));
 got.clear();api.IterateInventoryDirect(collect,nullptr);assert((got==std::vector<u16>{2,3}));
 reset();got.clear();api.IterateInventory([&](void*,void* p){auto* item=static_cast<CGameObject*>(p);got.push_back(item->ID());if(item->ID()==2){box.parent=nullptr;actor.inv.m_all.clear();}return false;},nullptr);assert((got==std::vector<u16>{2}));
 reset();got.clear();api.IterateInventory([&](void*,void* p){auto* item=static_cast<CGameObject*>(p);got.push_back(item->ID());if(item->ID()==2){host.Objects.live.erase(3);host.Objects.live.erase(4);actor.inv.m_all.clear();}return false;},nullptr);assert((got==std::vector<u16>{2}));
 reset();got.clear();api.IterateInventory([&](void*,void* p){auto* item=static_cast<CGameObject*>(p);got.push_back(item->ID());if(item->ID()==2){child.parent=&actor;actor.inv.m_all.push_back(&child);box.m_items.clear();}return false;},nullptr);assert((got==std::vector<u16>{2,3,4}));
 reset();int calls=0;api.IterateInventory([&](void*,void*){++calls;return true;},nullptr);assert(calls==1);
 reset();got.clear();api.ForEachInventoryItems([&](void* p,void*){auto* item=static_cast<CGameObject*>(p);got.push_back(item->ID());if(item->ID()==2){box.parent=nullptr;actor.inv.m_all.clear();}return false;});assert((got==std::vector<u16>{2}));
 reset();CScriptGameObject box_api{&box};box.m_items={4,5};stale.parent=&box;got.clear();box_api.IterateContainer([&](void*,void* p){got.push_back(static_cast<CGameObject*>(p)->ID());stale.parent=nullptr;return false;},nullptr);assert((got==std::vector<u16>{4}));
 reset();box.m_items.clear();assert(!actor.inv.AmpInCarriedBox(&child));
 puts("PASS: 10 engine-source scenarios: automatic contents, direct capacity, stale ownership, dropped/destroyed boxes, callback transfers, early stop, quest iteration and container iteration.");
}
"""
build=Path(workspace.name)
p=build/'inventory_access_test.cpp';p.write_text(source)
with (build/'inventory_access_compile.log').open('w') as log:
    subprocess.run([args.compiler]+(['c++'] if Path(args.compiler).stem=='zig' else [])+['-std=c++17','-Wall','-Wextra','-Werror',str(p),'-o',str(build/'inventory_access_test.exe')],stdout=log,stderr=log,check=True)
r=subprocess.run([str(build/'inventory_access_test.exe')],capture_output=True,text=True,check=True)
print(r.stdout)
workspace.cleanup()
