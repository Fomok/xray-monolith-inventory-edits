from pathlib import Path
import argparse,subprocess,tempfile
p=argparse.ArgumentParser();p.add_argument('--compiler',required=True);args=p.parse_args()
E=Path(__file__).resolve().parents[1]
def function(s,name):
 start=s.index(name);b=s.index('{',start);depth=1;i=b+1
 while depth: depth+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[start:i]
clip=function((E/'src/xrGame/ui_base.cpp').read_text(encoding='utf-8'),'sPoly2D* C2DFrustum::ClipPoly(')
a=clip.index('    // Ordinary rectangular');b=clip.index('\n\tfor (u32 i = 0;',a)
reference=(clip[:a]+clip[b:]).replace('::ClipPoly(', '::Reference(')
static=(E/'src/xrGame/UIStaticItem.cpp').read_text(encoding='utf-8')
submit=function(static,'void CUIStaticItem::RenderPolygon(')
assert static.count('RenderPolygon(R);')==2
for sig in ['void CUIStaticItem::Render()','void CUIStaticItem::Render(float angle)']:
 assert 'StartPrimitive' not in function(static,sig) and 'FlushPrimitive' not in function(static,sig)
head=r'''
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <random>
using u32=unsigned; using BOOL=bool; constexpr bool TRUE=true,FALSE=false;
#define IC inline
#define VERIFY(x) assert(x)
#define UI_FRUSTUM_SAFE 48
const float EPS_S=0.0000001f;
bool negative(float v){return std::signbit(v);} bool positive(float v){return !std::signbit(v);}
struct Fvector2 {float x=0,y=0;
 Fvector2& set(float a,float b){x=a;y=b;return *this;}
 void sub(Fvector2 a,Fvector2 b){x=a.x-b.x;y=a.y-b.y;}
 bool similar(Fvector2 b,float e)const{return std::abs(x-b.x)<e&&std::abs(y-b.y)<e;}
 float dotproduct(Fvector2 b)const{return x*b.x+y*b.y;}
 void mad(Fvector2 p,Fvector2 d,float t){x=p.x+d.x*t;y=p.y+d.y*t;}
};
struct Frect {float x1=0,y1=0,x2=100,y2=100;bool in(Fvector2 p)const{return p.x>=x1&&p.x<=x2&&p.y>=y1&&p.y<=y2;}};
struct Fplane2 {Fvector2 n;float d;float classify(Fvector2 p)const{return n.dotproduct(p)+d;}};
struct S2DVert {Fvector2 pt,uv;};
'''
# Actual engine fixed vector ensures clipped-list scratch semantics are identical.
head+=(E/'src/xrCore/FixedVector.h').read_text().replace('#pragma once','')
head+=r'''
using sPoly2D=svector<S2DVert,48>;
struct C2DFrustum {
 std::vector<Fplane2> planes={{{-1,0},0},{{0,-1},0},{{1,0},-100},{{0,1},-100}};
 Frect m_rect;bool m_force_clip=false;
 sPoly2D* ClipPoly(sPoly2D&,sPoly2D&)const;
 sPoly2D* Reference(sPoly2D&,sPoly2D&)const;
};
struct IUIRender {enum {ptTriList,pttTL,pttLIT};};
struct Core{int m_currentPointType=IUIRender::pttTL;} core;
Core& UI(){return core;}
struct Renderer {
 int locks=0,flushes=0;u32 budget=0;std::vector<S2DVert> vertices;
 void StartPrimitive(u32 n,int,int){++locks;budget=n;vertices.clear();}
 void PushPoint(float x,float y,float z,u32 color,float u,float v){assert(z==0&&color==0xaabbccdd);vertices.push_back({{x,y},{u,v}});}
 void FlushPrimitive(){++flushes;assert(vertices.size()==budget);}
} renderer;
Renderer* UIRender=&renderer;
struct CUIStaticItem {u32 dwColor=0xaabbccdd;void RenderPolygon(const sPoly2D*);};
'''
tail=r'''
void compare(const C2DFrustum& f,sPoly2D input){
 auto a=input,b=input;sPoly2D da,db;auto pa=f.ClipPoly(a,da),pb=f.Reference(b,db);
 assert(bool(pa)==bool(pb));if(!pa)return;assert(pa->size()==pb->size());
 for(u32 i=0;i<pa->size();++i){assert((*pa)[i].pt.similar((*pb)[i].pt,1e-5));assert((*pa)[i].uv.similar((*pb)[i].uv,1e-5));}
 CUIStaticItem item;for(int kind:{IUIRender::pttTL,IUIRender::pttLIT}){
 core.m_currentPointType=kind;int old=renderer.locks;item.RenderPolygon(pa);assert(renderer.locks==old+1);
 u32 v=0;for(u32 k=1;k+1<pa->size();++k)for(u32 idx:{0u,k,k+1}){
 assert(renderer.vertices[v].pt.similar((*pa)[idx].pt,1e-5));assert(renderer.vertices[v].uv.similar((*pa)[idx].uv,1e-5));++v;}
 }
}
int main(){
 CUIStaticItem item;sPoly2D empty;item.RenderPolygon(nullptr);item.RenderPolygon(&empty);empty.resize(2);item.RenderPolygon(&empty);assert(renderer.locks==0&&renderer.flushes==0);
 std::mt19937 rng(42);std::uniform_real_distribution<float> pos(-200,200),sz(0.1,150),ang(-3.15,3.15);
 for(int i=0;i<25000;++i){
 float x=pos(rng),y=pos(rng),w=sz(rng),h=sz(rng),angle=ang(rng);sPoly2D p;
 for(Fvector2 v: {Fvector2{0,0},Fvector2{w,0},Fvector2{w,h},Fvector2{0,h}}){p.push_back({{x+v.x*std::cos(angle)-v.y*std::sin(angle),y+v.x*std::sin(angle)+v.y*std::cos(angle)},{v.x/w,v.y/h}});}
 C2DFrustum f;compare(f,p);f.m_force_clip=true;f.planes.push_back({{0.70710678f,0.70710678f},-80});compare(f,p);
 }
 for(float x:{-100.f,0.f,100.f,101.f}){sPoly2D p;p.push_back({{x,0},{0,0}});p.push_back({{x+100,0},{1,0}});p.push_back({{x+100,100},{1,1}});p.push_back({{x,100},{0,1}});compare(C2DFrustum{},p);}
 printf("PASS: 50000 clipped polygon comparisons, custom planes, rotated geometry, edges, UVs, exact vertex budgets and invisible submission\n");
}
'''
with tempfile.TemporaryDirectory(prefix='sqa-render-') as d:
 src=Path(d)/'render.cpp';exe=Path(d)/'render.exe';src.write_text(head+clip+'\n'+reference+'\n'+submit+'\n'+tail)
 cmd=[args.compiler]+(['c++'] if Path(args.compiler).stem=='zig' else [])+['-std=c++17',str(src),'-o',str(exe)]
 result=subprocess.run(cmd,capture_output=True,text=True);assert result.returncode==0,result.stderr[-5000:]
 subprocess.run([str(exe)],check=True)
