from pathlib import Path
import re, subprocess, tempfile
import argparse
parser=argparse.ArgumentParser();parser.add_argument('--compiler',required=True);args=parser.parse_args()
engine=Path(__file__).resolve().parents[1]
root=engine.parent
def function(text,name):
    start=text.index(name)
    brace=text.index('{',start); depth=1; end=brace+1
    while depth:
        depth += (text[end]=='{')-(text[end]=='}'); end+=1
    return text[start:end]
cpp=(engine/'src/xrGame/ui/UIWindow.cpp').read_text(encoding='utf-8')
h=(engine/'src/xrGame/ui/UIWindow.h').read_text(encoding='utf-8')
reparent=function(cpp,'bool CUIWindow::Reparent(')
hit=function(h,'IC bool HitClipPass(').replace('IC bool','bool')
draw=function(cpp,'void CUIWindow::Draw()')
# Match the production rectangle overloads, including their reference qualifiers.
rect_header=(engine/'src/xrCore/_rect.h').read_text(encoding='utf-8')
rect_methods=re.findall(r'IC BOOL in\([^\n]+',rect_header)
assert len(rect_methods)==2
rect_methods='\n'.join(rect_methods).replace('IC BOOL','bool').replace('Tvector','Fvector2')
rect_methods=re.sub(r'\bT\b','float',rect_methods)
head=r'''
#include <vector>
#include <algorithm>
#include <cassert>
#include <cstdio>
using u32=unsigned;
#define PROF_EVENT(x)
struct Fvector2 {float x,y;};
struct Frect {float x1=0,y1=0,x2=100,y2=100; RECT_METHODS };
struct Render {int depth=0,pushes=0;void PushScissor(Frect){++depth;++pushes;}void PopScissor(){assert(depth>0);--depth;}} renderer;
Render& UI(){return renderer;}
struct xrCriticalSectionGuard {xrCriticalSectionGuard(int){}};
struct CUIWindow {
 CUIWindow* parent=nullptr;std::vector<CUIWindow*> m_ChildWndList,pending;
 bool autoDelete=false,shown=true,enabled=true,m_clip_children=false,custom=false;
 std::vector<Fvector2> m_hit_clip_poly;Frect rect;int csUi=0,draws=0,drawDepth=0;
 virtual ~CUIWindow()=default;
 CUIWindow* GetParent()const{return parent;}
 bool IsAutoDelete(){return autoDelete;}void SetAutoDelete(bool v){autoDelete=v;}
 bool IsShown(){return shown;}bool IsEnabled(){return enabled;}bool GetCustomDraw(){return custom;}
 void GetAbsoluteRect(Frect& r){r=rect;}
 void AttachChild(CUIWindow* w){assert(!w->parent);w->parent=this;m_ChildWndList.push_back(w);}
 void DetachChild(CUIWindow* w){auto it=std::find(m_ChildWndList.begin(),m_ChildWndList.end(),w);assert(it!=m_ChildWndList.end());m_ChildWndList.erase(it);w->parent=nullptr;if(w->autoDelete)pending.push_back(w);}
 bool Reparent(CUIWindow*);
 virtual void Draw();
'''
tail=r'''
};
struct Child : CUIWindow {void Draw()override{++draws;drawDepth=UI().depth;}};
int main(){
 CUIWindow a,b,c,leaf;
 a.AttachChild(&c);c.AttachChild(&leaf);c.SetAutoDelete(true);
 assert(c.Reparent(&b));assert(c.GetParent()==&b&&c.IsAutoDelete());assert(a.pending.empty()&&a.m_ChildWndList.empty());
 assert(c.Reparent(&b)&&b.m_ChildWndList.size()==1);
 assert(!c.Reparent(&leaf)&&!c.Reparent(&c)&&!c.Reparent(nullptr));assert(c.GetParent()==&b);
 c.SetAutoDelete(false);assert(c.Reparent(&a)&&!c.IsAutoDelete());assert(b.pending.empty());
 a.m_clip_children=true;a.rect={0,0,100,100};c.m_clip_children=true;c.rect={20,20,80,80};
 assert(leaf.HitClipPass({50,50}));assert(!leaf.HitClipPass({10,50}));assert(!leaf.HitClipPass({110,50}));
 c.shown=false;assert(!leaf.HitClipPass({50,50}));c.shown=true;c.enabled=false;assert(!leaf.HitClipPass({50,50}));c.enabled=true;
 c.rect={40,40,60,60};assert(!leaf.HitClipPass({30,50}));
 leaf.m_hit_clip_poly={{45,45},{55,45},{55,55},{45,55}};assert(leaf.HitClipPass({50,50}));assert(!leaf.HitClipPass({41,50}));
 CUIWindow host;Child item,hidden,custom;host.AttachChild(&item);host.AttachChild(&hidden);host.AttachChild(&custom);hidden.shown=false;custom.custom=true;
 host.m_clip_children=true;host.Draw();assert(item.draws==1&&item.drawDepth==1&&hidden.draws==0&&custom.draws==0&&UI().depth==0);
 host.m_clip_children=false;host.Draw();assert(item.drawDepth==0&&UI().pushes==1);
 puts("PASS: native UI reparent ownership, cycle rejection, nested clipping, hidden/disabled parents, polygon compatibility, scissor balance and default behavior");
}
'''
with tempfile.TemporaryDirectory(prefix='sqa-ui-') as tmp:
    src=Path(tmp)/'ui.cpp'; exe=Path(tmp)/'ui.exe';source=head.replace('RECT_METHODS',rect_methods)+hit+tail.split('int main()')[0]+reparent+'\n'+draw+'\nint main()'+tail.split('int main()')[1]
    src.write_text(source.replace('rect.in(abs_pos.x, abs_pos.y)','rect.in(abs_pos)'))
    command=[args.compiler]+(['c++'] if Path(args.compiler).stem=='zig' else [])+['-std=c++17',str(src),'-o',str(exe)]
    baseline=subprocess.run(command,capture_output=True,text=True)
    assert baseline.returncode!=0 and 'const' in baseline.stderr, 'Expected original const-reference compile failure'
    print('PASS: original build failure reproduced with production rectangle overloads')
    src.write_text(source)
    result=subprocess.run([args.compiler]+(['c++'] if Path(args.compiler).stem=='zig' else [])+['-std=c++17',str(src),'-o',str(exe)],capture_output=True,text=True)
    assert result.returncode==0,result.stderr[-3000:]
    subprocess.run([str(exe)],check=True)
