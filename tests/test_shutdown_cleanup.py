from pathlib import Path
import argparse,subprocess,tempfile
parser=argparse.ArgumentParser();parser.add_argument('--compiler',required=True);args=parser.parse_args()
engine=Path(__file__).resolve().parents[1]
def extract(s):
 a=s.index('void xrServer::Perform_destroy(');b=s.index('\nvoid xrServer::SLS_Clear()',a);return s[a:b]
head=r"""
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <cstdio>
using u16=unsigned short;using u32=unsigned;
#define R_ASSERT(c) do {if(!(c)) throw std::runtime_error("assertion");} while(0)
#define R_ASSERT2(c,m) R_ASSERT(c)
#define Msg(...) ((void)0)
const int NET_Latency=1,M_EVENT=1,GE_DESTROY=2,BroadcastCID=0;
struct {unsigned dwTimeGlobal=100;} Device;
struct NET_Packet {void w_begin(int){} void w_u32(unsigned){} void w_u16(unsigned short){}};
struct CSE_Abstract {u16 ID,ID_Parent;std::vector<u16> children;};
struct Game {std::map<u16,CSE_Abstract*> live;CSE_Abstract* get_entity_from_eid(u16 id){auto i=live.find(id);return i==live.end()?nullptr:i->second;}};
struct xrServer {Game* game;std::vector<u16> destroyed;void Perform_destroy(CSE_Abstract*,u32);
 void Perform_reject(CSE_Abstract* item,CSE_Abstract* parent,int){R_ASSERT(item->ID_Parent==parent->ID);auto i=std::find(parent->children.begin(),parent->children.end(),item->ID);R_ASSERT(i!=parent->children.end());parent->children.erase(i);item->ID_Parent=0xffff;}
 void entity_Destroy(CSE_Abstract* item){R_ASSERT(game->live.erase(item->ID)==1);destroyed.push_back(item->ID);}
 void SendBroadcast(int,NET_Packet&,u32){}
};
"""
tail=r"""
int main(){try{
 {Game g;xrServer s{&g,{}};CSE_Abstract root{1,0xffff,{3699}};g.live[1]=&root;s.Perform_destroy(&root,0);R_ASSERT(g.live.empty());}
 {Game g;xrServer s{&g,{}};CSE_Abstract root{1,0xffff,{2,2}},child{2,1,{}};g.live={{1,&root},{2,&child}};s.Perform_destroy(&root,0);R_ASSERT((s.destroyed==std::vector<u16>{2,1}));}
 {Game g;xrServer s{&g,{}};CSE_Abstract root{1,0xffff,{2}},child{2,3,{}},other{3,0xffff,{2}};g.live={{1,&root},{2,&child},{3,&other}};s.Perform_destroy(&root,0);R_ASSERT(g.live.size()==2 && child.ID_Parent==3 && other.children.size()==1);s.Perform_destroy(&other,0);R_ASSERT(g.live.empty());}
 {Game g;xrServer s{&g,{}};CSE_Abstract root{1,0xffff,{2}},box{2,1,{3}},item{3,2,{}};g.live={{1,&root},{2,&box},{3,&item}};s.Perform_destroy(&root,0);R_ASSERT((s.destroyed==std::vector<u16>{3,2,1}));}
 puts("PASS: missing child, duplicate child, reparented child and normal nested cleanup");return 0;
}catch(const std::exception&){puts("cleanup assertion reproduced");return 1;}}
"""
path='src/xrGame/xrServer_sls_clear.cpp'
fixed=(engine/path).read_text(encoding='utf-8-sig')
# The tested Preview 1 commit contains the original failing implementation.
old=subprocess.run(['git','show','effb7a2:'+path],cwd=engine,capture_output=True,text=True,check=True).stdout
with tempfile.TemporaryDirectory(prefix='sqa-shutdown-') as tmp:
 for label,source,expected in [('baseline',old,1),('fixed',fixed,0)]:
  cpp=Path(tmp)/(label+'.cpp');exe=Path(tmp)/(label+'.exe');cpp.write_text(head+extract(source)+tail)
  cmd=[args.compiler]+(['c++'] if Path(args.compiler).stem=='zig' else [])+['-std=c++17',str(cpp),'-o',str(exe)]
  result=subprocess.run(cmd,capture_output=True,text=True)
  if result.returncode: raise RuntimeError(result.stderr)
  result=subprocess.run([str(exe)],capture_output=True,text=True)
  print(label+': '+result.stdout.strip());assert result.returncode==expected
