"""Compile the production layout and actual CSE STATE methods with a bounded packet host."""
from pathlib import Path
import argparse,subprocess
parser=argparse.ArgumentParser();parser.add_argument('--compiler',required=True);args=parser.parse_args()
E=Path(__file__).resolve().parents[1];B=E.parent/'build';B.mkdir(exist_ok=True)
s=(E/'src/xrServerEntities/xrServer_Objects_ALife_Items.cpp').read_text(encoding="utf-8-sig")
a=s.index('void CSE_ALifeInventoryItem::STATE_Write');b=s.index('\nstatic inline bool check(',a)
methods=s[a:b]
h=(E/'src/xrServerEntities/xrServer_Objects_ALife_Items.h').read_text(encoding="utf-8-sig");a=h.index('    inventory_layout::Placement m_inventory_layout;');b=h.index('\npublic:\n\t//  LIMITS',a)
api=h[a:b]
head=r"""
#include <vector>
#include <stdexcept>
#include <cstring>
#include <cstdio>
#include <climits>
#include "../engine/src/xrServerEntities/inventory_membership.h"
using u8=unsigned char;using u16=unsigned short;using LPCSTR=const char*;
#define CHECK(v) do { if(!(v)) throw std::runtime_error(#v); } while(0)
#define R_ASSERT2(v,msg) CHECK(v)
struct NET_Packet {
 std::vector<u8> data;size_t pos=0;
 void w_u8(unsigned v){data.push_back(static_cast<u8>(v));}
 void w_u16(unsigned v){w_u8(v);w_u8(v>>8);}
 unsigned r_u8(){if(pos>=data.size()) throw std::runtime_error("packet underflow");return data[pos++];}
 unsigned r_u16(){unsigned a=r_u8();return a|(r_u8()<<8);}
 void w_float(float f){u8 bytes[4];std::memcpy(bytes,&f,4);for(auto b:bytes) w_u8(b);}
 void r_float(float& f){u8 bytes[4];for(auto& b:bytes)b=static_cast<u8>(r_u8());std::memcpy(&f,bytes,4);}
};
void save_data(const std::vector<u16>& v,NET_Packet& p){p.w_u16(v.size());for(auto x:v)p.w_u16(x);}
void load_data(std::vector<u16>& v,NET_Packet& p){v.clear();unsigned n=p.r_u16();for(unsigned i=0;i<n;++i)v.push_back(p.r_u16());}
struct CSE_ALifeInventoryItem {
 struct Base{u16 m_wVersion=131;int o_Position=17;} self;
 struct{int position=0;}State;float m_fCondition=0.5f;
 std::vector<u16>m_upgrades{21,34},m_item_data{56};
 Base* base(){return &self;}
 void STATE_Write(NET_Packet&);void STATE_Read(NET_Packet&,u16);
"""
tail=r"""
static bool same(const inventory_layout::Placement& a,const inventory_layout::Placement& b){return a.valid==b.valid&&a.x==b.x&&a.y==b.y&&a.width==b.width&&a.height==b.height&&a.rotated==b.rotated&&a.manual==b.manual;}
int main(){try{
 using inventory_layout::Placement;
 Placement p;CHECK(!p.valid);NET_Packet empty;p.write(empty);CHECK(empty.data.size()==1);CHECK(p.set(1,2,3,4,true,true));CHECK(p.read(empty));CHECK(!p.valid);
 int count=0;
 for(int x:{0,1,4095}) for(int y:{0,1,4095}) for(int w:{1,3,4096}) for(int h:{1,2,4096}) for(bool rot:{false,true}) for(bool manual:{false,true}){
  CHECK(p.set(x,y,w,h,rot,manual));NET_Packet out;p.write(out);CHECK(out.data.size()==9);out.w_u16(0xbeef);Placement q;CHECK(q.read(out));CHECK(same(p,q));CHECK(out.r_u16()==0xbeef);++count;
 }
 auto before=p;
 CHECK(!p.set(-1,0,1,1,false,false));CHECK(!p.set(0,-1,1,1,false,false));CHECK(!p.set(4096,0,1,1,false,false));CHECK(!p.set(0,4096,1,1,false,false));CHECK(!p.set(0,0,0,1,false,false));CHECK(!p.set(0,0,1,0,false,false));CHECK(!p.set(0,0,4097,1,false,false));CHECK(!p.set(INT_MAX,0,1,1,false,false));CHECK(same(p,before));
 for(unsigned flag:{2,4,6,8,255}){NET_Packet bad;bad.w_u8(flag);CHECK(!p.read(bad));CHECK(same(p,before));}
 NET_Packet zero;zero.w_u8(1);for(int i=0;i<4;++i)zero.w_u16(0);CHECK(!p.read(zero));CHECK(same(p,before));
 NET_Packet truncated;truncated.w_u8(1);bool failed=false;try{p.read(truncated);}catch(...){failed=true;}CHECK(failed&&same(p,before));
 for(bool valid:{false,true}){
  CSE_ALifeInventoryItem a,b;if(valid)CHECK(a.set_inventory_layout(2,8,1,3,true,true));NET_Packet packet;a.STATE_Write(packet);packet.w_u16(0xabcd);b.STATE_Read(packet,0);CHECK(packet.r_u16()==0xabcd);CHECK(same(a.inventory_layout(),b.inventory_layout()));CHECK(a.m_item_data==b.m_item_data&&a.m_upgrades==b.m_upgrades);CHECK(b.State.position==17);
  auto copy=b.inventory_layout();copy.clear();CHECK(b.inventory_layout().valid==valid); // Lua snapshot cannot mutate storage.
 }
 for(unsigned version:{123,128,129}){
  CSE_ALifeInventoryItem spawned;spawned.self.m_wVersion=version;spawned.set_inventory_layout(1,1,1,1,false,false);NET_Packet packet;packet.w_float(0.8f);if(version>123)save_data(spawned.m_upgrades,packet);if(version>128)save_data(spawned.m_item_data,packet);packet.w_u16(0xabcd);spawned.STATE_Read(packet,0);CHECK(!spawned.inventory_layout().valid);CHECK(packet.r_u16()==0xabcd);
 }
 printf("PASS: %d layout round trips, invalid writes/packets, CSE framing, stock spawn versions and detached getters\n",count);return 0;
}catch(const std::exception& e){puts(e.what());return 1;}}
"""
cpp=B/'inventory_layout_test.cpp';exe=B/'inventory_layout_test.exe';cpp.write_text(head+api+'\n};\n'+methods+tail)
subprocess.run([args.compiler,'c++','-std=c++17',str(cpp),'-o',str(exe)],check=True)
subprocess.run([str(exe)],check=True)
# A tiny bridge lets the Lua regression suite use the real native validation.
bridge=B/'inventory_layout_bridge.cpp';dll=B/'inventory_layout_bridge.dll'
bridge.write_text(r"""
#include "../engine/src/xrServerEntities/inventory_layout.h"
using inventory_layout::Placement;
extern "C" {
__declspec(dllexport) Placement* layout_new(){return new Placement;}
__declspec(dllexport) void layout_delete(Placement* p){delete p;}
__declspec(dllexport) int layout_set(Placement* p,int x,int y,int w,int h,int r,int m){return p->set(x,y,w,h,r!=0,m!=0);}
__declspec(dllexport) int layout_get(Placement* p,int i){switch(i){case 0:return p->valid;case 1:return p->x;case 2:return p->y;case 3:return p->width;case 4:return p->height;case 5:return p->rotated;default:return p->manual;}}
__declspec(dllexport) void layout_clear(Placement* p){p->clear();}
}
""")
subprocess.run([args.compiler,'c++','-std=c++17','-shared',str(bridge),'-o',str(dll)],check=True)
print('PASS: native bridge built for Lua persistence tests')

