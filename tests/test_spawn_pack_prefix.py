"""Compile the production spawn-pack prefix reader with bounded packet fixtures."""
from pathlib import Path
import argparse, re, subprocess, tempfile
p=argparse.ArgumentParser();p.add_argument('--compiler',required=True);a=p.parse_args()
r=Path(__file__).resolve().parents[1]
s=(r/'src/xrGame/spawn_packs.cpp').read_text()
h=(r/'src/xrServerEntities/xrServer_Objects.h').read_text()
version=re.search(r'#define SPAWN_VERSION\s+u16\((\d+)\)',h).group(1)
constants=s[s.index('\tconst u16 MIN_PEEKED'):s.index('\n\tstatic_assert(XRAI')]
helper=s[s.index('\tbool has_bytes('):s.index('\n\tLPCSTR pack_level_name(')]
source=r'''
#include <vector>
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <cstdio>
using u16=unsigned short;using u32=unsigned;
struct Fvector {float x,y,z;};
constexpr u16 M_SPAWN_VERSION=0x20;
struct NET_Packet {
 std::vector<unsigned char> b;size_t pos=0;
 u32 r_elapsed() const {return u32(b.size()-pos);}
 void r_advance(size_t n){if(n>r_elapsed())throw std::runtime_error("overread");pos+=n;}
 void r_vec3(Fvector& v){auto start=pos;r_advance(sizeof(v));memcpy(&v,b.data()+start,sizeof(v));}
 u16 r_u16(){auto start=pos;r_advance(2);return b[start]|(u16(b[start+1])<<8);}
 void word(u16 v){b.push_back(v&255);b.push_back(v>>8);}
};
'''+f'\n#define SPAWN_VERSION u16({version})\n'+constants+helper+r'''
NET_Packet fixture(u16 version,u16 client_size=0,u16 flags=M_SPAWN_VERSION){
 NET_Packet p;p.b.resize(sizeof(Fvector));
 for(int i=0;i<4;++i)p.word(100+i);
 p.word(flags);p.word(version);p.word(0);p.word(1);p.word(client_size);
 for(u16 i=0;i<client_size;++i)p.b.push_back(i%256);
 p.word(500);p.word(64);p.word(12345);return p;
}
int main(){
 for(u16 version=128;version<=133;++version){
  for(u16 size:{0,1,255,256,4096}){
   auto full=fixture(version,size);auto good=full;
   assert(peek_graph_vertex(good)==12345&&good.r_elapsed()==0);
   for(size_t n=0;n<full.b.size();++n){auto short_packet=full;short_packet.b.resize(n);assert(peek_graph_vertex(short_packet)==u32(-1));}
   full.b.insert(full.b.end(),64,0xff);assert(peek_graph_vertex(full)==12345&&full.r_elapsed()==64);
  }
 }
 for(u16 version:{0,93,127,134,65535}){auto p=fixture(version);assert(peek_graph_vertex(p)==u32(-1));}
 auto noflag=fixture(133,0,0);assert(peek_graph_vertex(noflag)==u32(-1));
 puts("PASS: production spawn-pack reader accepts stock 128 and Squared Away 129-133; variable client data, every truncated prefix, unknown versions, missing version flag and untouched item payload");
}
'''
with tempfile.TemporaryDirectory(prefix='sqa-spawn-pack-') as d:
 cpp=Path(d)/'test.cpp';exe=Path(d)/'test.exe';cpp.write_text(source)
 subprocess.run([a.compiler,'c++','-std=c++17',str(cpp),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
