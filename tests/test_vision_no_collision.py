from pathlib import Path
import argparse,subprocess,tempfile
parser=argparse.ArgumentParser();parser.add_argument('--compiler',required=True);args=parser.parse_args()
R=Path(__file__).resolve().parents[1];path='src/xrEngine/Feel_Vision.cpp'
def extract(s):
 a=s.index('void Vision::o_new(');return s[a:s.index('\nvoid Vision::o_delete',a)]
head=r'''
#include <vector>
#include <stdexcept>
#include <cstdio>
using u16=unsigned short;
const float EPS_S=0.000001f;
#define CHECK(x) do { if (!(x)) throw std::runtime_error(#x); } while (0)
struct Fvector {float x=0,y=0,z=0; void set(float a,float b,float c){x=a;y=b;z=c;} };
struct CObject {
 bool has_form=false;int samples=0,seeds=0;u16 bone=0xffff;Fvector position{5,6,7};
 const void* CFORM()const{return has_form?this:nullptr;}
 const Fvector& Position()const{return position;}
 Fvector get_new_local_point_on_mesh(u16& b){++seeds;b=bone;return {1,2,3};}
 Fvector get_last_local_point_on_mesh(const Fvector& p,u16 b){
  CHECK(CFORM()!=nullptr);++samples;CHECK(b==bone);return {p.x+10,p.y+20,p.z+30};
 }
};
struct xrSRWLockGuard{xrSRWLockGuard(int*,bool){}};
struct Vision {
 struct feel_visible_Item {CObject* O=nullptr;struct {Fvector verts[3];} Cache;float Cache_vis=0,fuzzy=0;Fvector cp_LP,cp_LAST;u16 bone_id=0;};
 int lock_visible=0;std::vector<feel_visible_Item> feel_visible;void o_new(CObject*);
};
'''
tail=r'''
int main(){try{
 Vision vision;
 CObject helper;vision.o_new(&helper);
 CHECK(helper.samples==0 && helper.seeds==1 && vision.feel_visible.size()==1);
 auto& item=vision.feel_visible.back();CHECK(item.O==&helper && item.fuzzy<0);
 CHECK(item.cp_LAST.x==5 && item.cp_LAST.y==6 && item.cp_LAST.z==7);
 CHECK(item.bone_id==0xffff && item.Cache_vis==1);
 // The retained sample/entry is usable if the collision form becomes available.
 helper.has_form=true;auto p=helper.get_last_local_point_on_mesh(item.cp_LP,item.bone_id);
 CHECK(p.x==11 && p.y==22 && p.z==33);
 CObject scrap;scrap.has_form=true;vision.o_new(&scrap);
 CHECK(scrap.samples==1 && vision.feel_visible.size()==2);
 CHECK(vision.feel_visible.back().cp_LAST.x==11);
 CObject skeletal;skeletal.has_form=true;skeletal.bone=7;vision.o_new(&skeletal);
 CHECK(skeletal.samples==1 && vision.feel_visible.back().bone_id==7);
 CHECK(vision.feel_visible[0].O==&helper && vision.feel_visible[1].O==&scrap);
 puts("PASS: no-form helper, retained entry, later form, ordinary mesh and skeletal sampling");return 0;
}catch(const std::exception& e){puts(e.what());return 1;}}
'''
old=subprocess.run(['git','show','HEAD:'+path],cwd=R,capture_output=True,text=True,check=True).stdout
fixed=(R/path).read_text(encoding='utf-8-sig')
assert 'if (0 == I->O->CFORM())' in fixed # existing trace exclusion remains
with tempfile.TemporaryDirectory(prefix='sqa-vision-') as tmp:
 for label,src,expected in [('baseline',old,1),('fixed',fixed,0)]:
  cpp=Path(tmp)/(label+'.cpp');exe=Path(tmp)/(label+'.exe');cpp.write_text(head+extract(src)+tail)
  result=subprocess.run([args.compiler,'c++','-std=c++17',str(cpp),'-o',str(exe)],capture_output=True,text=True)
  if result.returncode:raise RuntimeError(result.stderr)
  result=subprocess.run([str(exe)],capture_output=True,text=True)
  print(label+': '+result.stdout.strip());assert result.returncode==expected
