from pathlib import Path
import argparse,subprocess,json
p=argparse.ArgumentParser();p.add_argument('--compiler',required=True);args=p.parse_args()
E=Path(__file__).resolve().parents[1];B=E.parent/'build';B.mkdir(exist_ok=True)
s=(E/'src/xrServerEntities/xrServer_Objects_ALife_Items.cpp').read_text(encoding='utf-8-sig')
a=s.index('bool CSE_ALifeInventoryItem::set_rig_membership');b=s.index('\nstatic inline bool check(',a)
methods=s[a:b]
h=(E/'src/xrServerEntities/xrServer_Objects_ALife_Items.h').read_text(encoding="utf-8-sig");a=h.index('    inventory_layout::Placement m_inventory_layout;');b=h.index('\npublic:\n\t//  LIMITS',a)
api=h[a:b]
head='\n#include <vector>\n#include <stdexcept>\n#include <cstring>\n#include <cstdio>\n#include <climits>\n#include "../engine/src/xrServerEntities/inventory_membership.h"\n#include "../engine/src/xrServerEntities/inventory_pouches.h"\nusing u8=unsigned char;using u16=unsigned short;using LPCSTR=const char*;\n#define CHECK(v) do { if(!(v)) throw std::runtime_error(#v); } while(0)\n#define R_ASSERT2(v,msg) CHECK(v)\nstruct NET_Packet {\n std::vector<u8> data;size_t pos=0;\n void w_u8(unsigned v){data.push_back(static_cast<u8>(v));}\n void w_u16(unsigned v){w_u8(v);w_u8(v>>8);}\n unsigned r_u8(){if(pos>=data.size()) throw std::runtime_error("packet underflow");return data[pos++];}\n unsigned r_u16(){unsigned a=r_u8();return a|(r_u8()<<8);}\n void w_float(float f){u8 bytes[4];std::memcpy(bytes,&f,4);for(auto b:bytes) w_u8(b);}\n void r_float(float& f){u8 bytes[4];for(auto& b:bytes)b=static_cast<u8>(r_u8());std::memcpy(&f,bytes,4);}\n};\nvoid save_data(const std::vector<u16>& v,NET_Packet& p){p.w_u16(v.size());for(auto x:v)p.w_u16(x);}\nvoid load_data(std::vector<u16>& v,NET_Packet& p){v.clear();unsigned n=p.r_u16();for(unsigned i=0;i<n;++i)v.push_back(p.r_u16());}\ntemplate<class T,class U>T smart_cast(U* p){return dynamic_cast<T>(p);}\nstruct CSE_ALifeInventoryItem {\n virtual ~CSE_ALifeInventoryItem()=default;\n struct Base{u16 m_wVersion=133;int o_Position=17;} self;\n struct{int position=0;}State;float m_fCondition=0.5f;\n std::vector<u16>m_upgrades{21,34},m_item_data{56};\n Base* base(){return &self;}\n void STATE_Write(NET_Packet&);void STATE_Read(NET_Packet&,u16);\n'
source=head+api+'\n};\nstruct CSE_ALifeItemContainer:CSE_ALifeInventoryItem {};\n'+methods
bridge=r'''
extern "C" {
__declspec(dllexport) CSE_ALifeInventoryItem* membership_new(int container){return container?new CSE_ALifeItemContainer:new CSE_ALifeInventoryItem;}
__declspec(dllexport) void membership_delete(CSE_ALifeInventoryItem* p){delete p;}
__declspec(dllexport) int membership_set(CSE_ALifeInventoryItem* p,CSE_ALifeInventoryItem* rig,const char* key,int x,int y,int w,int h,int rot,int rank){return p->set_rig_membership(rig,key,x,y,w,h,rot!=0,rank);}
__declspec(dllexport) int membership_matches(CSE_ALifeInventoryItem* p,CSE_ALifeInventoryItem* rig){return p->rig_membership_matches(rig);}
__declspec(dllexport) void membership_clear(CSE_ALifeInventoryItem* p){p->clear_rig_membership();}
__declspec(dllexport) int membership_get(CSE_ALifeInventoryItem* p,int i){auto m=p->rig_membership();switch(i){case 0:return m.valid();case 1:return m.layout.x;case 2:return m.layout.y;case 3:return m.layout.width;case 4:return m.layout.height;case 5:return m.layout.rotated;default:return m.order;}}
__declspec(dllexport) const char* membership_key(CSE_ALifeInventoryItem* p){static std::string key;key=p->rig_membership().pocket;return key.c_str();}
__declspec(dllexport) const char* pouches_get(CSE_ALifeInventoryItem* p,int slot){return p->rig_pouch(slot);}
__declspec(dllexport) int pouches_ready(CSE_ALifeInventoryItem* p){return p->rig_pouches_initialized();}
__declspec(dllexport) int pouches_set(CSE_ALifeInventoryItem* p,int slot,const char* section){return p->set_rig_pouch(slot,section);}
__declspec(dllexport) int pouches_replace(CSE_ALifeInventoryItem* p,const char* a,const char* b){return p->set_rig_pouches(a,b);}
__declspec(dllexport) int box_set(CSE_ALifeInventoryItem* p,CSE_ALifeInventoryItem* box,int x,int y,int w,int h,int rot,int rank){return p->set_box_layout(box,x,y,w,h,rot!=0,rank);}
__declspec(dllexport) int box_matches(CSE_ALifeInventoryItem* p,CSE_ALifeInventoryItem* box){return p->box_layout_matches(box);}
__declspec(dllexport) void box_clear(CSE_ALifeInventoryItem* p){p->clear_box_layout();}
__declspec(dllexport) int box_get(CSE_ALifeInventoryItem* p,int i){auto m=p->box_layout();switch(i){case 0:return m.valid();case 1:return m.layout.x;case 2:return m.layout.y;case 3:return m.layout.width;case 4:return m.layout.height;case 5:return m.layout.rotated;default:return m.order;}}
__declspec(dllexport) void membership_load_copy(CSE_ALifeInventoryItem* p,CSE_ALifeInventoryItem* source){NET_Packet packet;source->STATE_Write(packet);p->STATE_Read(packet,0);CHECK(packet.pos==packet.data.size());}
}
'''
tests=r'''
int main(){try{
 CSE_ALifeItemContainer rig,spare;CSE_ALifeInventoryItem item,other;
 CHECK(!item.set_rig_membership(&other,"b1",0,0,1,1,false,1));
 CHECK(!rig.set_rig_membership(&rig,"b1",0,0,1,1,false,1));
 CHECK(!item.set_rig_membership(nullptr,"b1",0,0,1,1,false,1));
 for(const char* key:{"b1","p1_2","p2_123"})for(bool rot:{false,true}){
  CHECK(item.set_rig_membership(&rig,key,2,3,1,2,rot,65535));CHECK(item.rig_membership_matches(&rig));CHECK(!item.rig_membership_matches(&spare));
  auto copy=item.rig_membership();copy.clear();CHECK(item.rig_membership().valid());
  NET_Packet a,b;rig.STATE_Write(a);item.STATE_Write(b);a.w_u16(0xabcd);b.w_u16(0xabcd);
  inventory_membership::watermark()=0;CSE_ALifeInventoryItem loaded;CSE_ALifeItemContainer loadedrig;
  loaded.STATE_Read(b,0);loadedrig.STATE_Read(a,0);CHECK(b.r_u16()==0xabcd&&a.r_u16()==0xabcd);
  CHECK(loaded.rig_membership_matches(&loadedrig));CHECK(loaded.rig_membership().layout.rotated==rot);CHECK(loaded.rig_membership().pocket==key);
  CSE_ALifeItemContainer newrig;CSE_ALifeInventoryItem newitem;CHECK(newitem.set_rig_membership(&newrig,"b1",0,0,1,1,false,0));CHECK(!loaded.rig_membership_matches(&newrig));
 }
 CSE_ALifeItemContainer pouched,loadedpouched;CSE_ALifeInventoryItem plain;
 CHECK(!pouched.rig_pouches_initialized());CHECK(!plain.set_rig_pouch(1,"af_magpouch_l"));
 CHECK(pouched.set_rig_pouches("af_magpouch_l","af_magpouch_m"));
 CHECK(!pouched.set_rig_pouches("other","bad section"));CHECK(std::string(pouched.rig_pouch(1))=="af_magpouch_l");
 for(int slot:{0,3,-1})CHECK(!pouched.set_rig_pouch(slot,"pouch"));
 CHECK(!pouched.set_rig_pouch(1,std::string(128,'a').c_str()));CHECK(!pouched.set_rig_pouch(1,nullptr));
 for(bool removed:{false,true}){
  if(removed)CHECK(pouched.set_rig_pouches("",""));
  NET_Packet packet;pouched.STATE_Write(packet);packet.w_u16(0xbeef);loadedpouched.STATE_Read(packet,0);
  CHECK(packet.r_u16()==0xbeef&&loadedpouched.rig_pouches_initialized());
  CHECK(std::string(loadedpouched.rig_pouch(1))==(removed?"":"af_magpouch_l"));
  CHECK(std::string(loadedpouched.rig_pouch(2))==(removed?"":"af_magpouch_m"));
 }
 // Version131 is the currently released save format, before native attachments.
 {CSE_ALifeItemContainer old;old.self.m_wVersion=131;NET_Packet p;p.w_float(1);save_data(old.m_upgrades,p);save_data(old.m_item_data,p);old.m_inventory_layout.write(p);inventory_membership::write_identity(p,0);inventory_membership::Membership{}.write(p);p.w_u16(0xbeef);old.STATE_Read(p,0);CHECK(!old.rig_pouches_initialized()&&p.r_u16()==0xbeef);}
 inventory_pouches::Attachments at;CHECK(at.replace("af_magpouch_l","af_magpouch_m"));NET_Packet bytes;at.write(bytes);
 for(size_t size=0;size<bytes.data.size();++size){NET_Packet truncated;truncated.data.assign(bytes.data.begin(),bytes.data.begin()+size);bool failed=false;try{failed=!at.read(truncated);}catch(...){failed=true;}CHECK(failed);CHECK(std::string(at.get(1))=="af_magpouch_l");}
 for(unsigned flag:{2,4,6,8,255}){NET_Packet bad;bad.w_u8(flag);CHECK(!at.read(bad));}
 {NET_Packet bad;bad.w_u8(3);bad.w_u8(128);CHECK(!at.read(bad));}
 puts("PASS: actual native pouch API, atomic replacement, both slots, explicit removals, v131 framing, invalid values and truncated records");

 CSE_ALifeItemContainer box,loadedbox,reusedbox;CSE_ALifeInventoryItem cargo,loadedcargo;
 CHECK(cargo.set_inventory_layout(9,8,1,2,false,true));CHECK(cargo.set_rig_membership(&rig,"b1",1,2,1,2,false,1));
 CHECK(cargo.set_box_layout(&box,3,4,1,2,true,7));
 CHECK(!cargo.set_box_layout(&cargo,0,0,1,1,false,0));CHECK(!cargo.set_box_layout(&other,0,0,1,1,false,0));
 CHECK(!cargo.set_box_layout(&box,-1,0,1,1,false,0));CHECK(!cargo.set_box_layout(&box,0,0,1,1,false,65536));
 CHECK(cargo.box_layout().layout.x==3&&cargo.box_layout().order==7);
 NET_Packet bp,cp;box.STATE_Write(bp);cargo.STATE_Write(cp);cp.w_u16(0xabcd);
 loadedcargo.STATE_Read(cp,0);loadedbox.STATE_Read(bp,0);
 CHECK(cp.r_u16()==0xabcd&&loadedcargo.box_layout_matches(&loadedbox)&&!loadedcargo.box_layout_matches(&reusedbox));
 CHECK(loadedcargo.box_layout().layout.rotated&&loadedcargo.box_layout().order==7);
 CHECK(loadedcargo.inventory_layout().x==9&&loadedcargo.rig_membership_matches(&rig));
 loadedcargo.clear_box_layout();CHECK(!loadedcargo.box_layout().valid()&&loadedcargo.inventory_layout().valid&&loadedcargo.rig_membership().valid());
 {CSE_ALifeInventoryItem old;old.self.m_wVersion=132;NET_Packet p; p.w_float(1);save_data(old.m_upgrades,p);save_data(old.m_item_data,p);old.m_inventory_layout.write(p);inventory_membership::write_identity(p,0);inventory_membership::Membership{}.write(p);inventory_pouches::Attachments{}.write(p);p.w_u16(0xbeef);old.STATE_Read(p,0);CHECK(!old.box_layout().valid()&&p.r_u16()==0xbeef);}
 puts("PASS: independent native box layout, owner identity, rotated coordinates, order, clear, rejected writes and v132 framing");
 auto before=item.rig_membership();for(const char* key:{"","bad key","bad:key","bad/key"})CHECK(!item.set_rig_membership(&rig,key,0,0,1,1,false,0));
 CHECK(!item.set_rig_membership(&rig,std::string(64,'b').c_str(),0,0,1,1,false,0));CHECK(!item.set_rig_membership(&rig,"b1",-1,0,1,1,false,0));CHECK(!item.set_rig_membership(&rig,"b1",0,0,1,1,false,65536));CHECK(item.rig_membership().pocket==before.pocket);
 for(unsigned version:{123,128,129,130}){
  CSE_ALifeInventoryItem old;old.self.m_wVersion=version;NET_Packet p;p.w_float(1);if(version>123)save_data(old.m_upgrades,p);if(version>128)save_data(old.m_item_data,p);if(version>=130)old.m_inventory_layout.write(p);p.w_u16(0xbeef);old.STATE_Read(p,0);CHECK(!old.rig_membership().valid()&&p.r_u16()==0xbeef);
 }
 inventory_membership::Membership m;CHECK(m.set(77,"b1",0,0,1,2,true,2));
 NET_Packet good;m.write(good);for(size_t size=0;size<good.data.size();++size){NET_Packet shortp;shortp.data.assign(good.data.begin(),good.data.begin()+size);bool rejected=false;try{rejected=!m.read(shortp);}catch(...){rejected=true;}CHECK(rejected);}
 for(unsigned flag:{2,255}){NET_Packet bad;bad.w_u8(flag);CHECK(!m.read(bad));}
 NET_Packet bad;bad.w_u8(1);inventory_membership::write_identity(bad,77);bad.w_u8(64);CHECK(!m.read(bad));
 inventory_membership::watermark()=(inventory_membership::Identity(1)<<60)+99;
 CSE_ALifeItemContainer highrig,loadedhighrig;CSE_ALifeInventoryItem highitem,loadedhighitem;
 CHECK(highitem.set_rig_membership(&highrig,"p1_2",4095,4095,4096,4096,true,65535));
 NET_Packet higha,highb;highrig.STATE_Write(higha);highitem.STATE_Write(highb);inventory_membership::watermark()=0;
 loadedhighitem.STATE_Read(highb,0);loadedhighrig.STATE_Read(higha,0);CHECK(loadedhighitem.rig_membership_matches(&loadedhighrig));
 CHECK(inventory_membership::watermark()>(inventory_membership::Identity(1)<<60));
 item.clear_rig_membership();CHECK(!item.rig_membership_matches(&rig));
 puts("PASS: actual CSE membership API and serialization, identities, read-order independence, invalid writes, bounds, old spawn framing and truncated packets");return 0;
}catch(const std::exception& e){puts(e.what());return 1;}}
'''
cpp=B/'inventory_membership_test.cpp';exe=B/'inventory_membership_test.exe'
cpp.write_text(source+bridge+tests)
subprocess.run([args.compiler,'c++','-std=c++17',str(cpp),'-o',str(exe)],check=True);subprocess.run([str(exe)],check=True)
cpp=B/'inventory_membership_bridge.cpp';dll=B/'inventory_membership_bridge.dll';cpp.write_text(source+bridge)
subprocess.run([args.compiler,'c++','-std=c++17','-shared',str(cpp),'-o',str(dll)],check=True)
print('PASS: compiled production membership bridge for Lua integration')
