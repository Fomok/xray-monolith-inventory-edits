#include "drag_session.h"
#include <iostream>
#include <stdexcept>
using namespace inventory_next;
void check(bool ok,const char* text) { if(!ok) throw std::runtime_error(text); }
Node root(Id id,Kind kind,Size grid) { Node n; n.id=id; n.kind=kind; n.category=0; n.container=true; n.grid=grid; return n; }
Node item(Id id,Id parent,Size size,Position position) { Node n; n.id=id; n.parent=parent; n.intrinsic=20; n.empty=n.half=n.full=size; n.position=position; return n; }
int main() {
 try {
    Model m; auto r=m.initialize({root(1,Kind::ActorRoot,{3,3}),root(2,Kind::WorldRoot,{3,3}),item(10,1,{2,1},{}),item(11,2,{2,1},{})});
    check(bool(r),r.error.c_str()); DragSession drag; const auto before=m.serialize();
    check(!drag.begin(m,1)&&!drag.begin(m,999),"invalid origin"); check(bool(drag.begin(m,10)),"begin");
    auto p=drag.preview(m,1,0,1,false,{}); check(p.feedback==DropFeedback::Valid&&p.moves.size()==1,"green fit");
    p=drag.preview(m,1,2,2,false,{}); check(p.feedback==DropFeedback::Invalid&&p.moves.empty(),"red bounds");
    check(m.serialize()==before,"preview mutation");
    std::cout<<"PASS valid invalid previews and no mutation\n";
    check(drag.rotate(m),"rotate"); p=drag.preview(m,1,2,0,false,{});
    check(p.feedback==DropFeedback::Valid&&p.footprint.w==1&&p.footprint.h==2&&p.position.rotated,"rotated ghost");
    p=drag.release(m,1,2,0,false,{}); check(!drag.item()&&m.serialize()==before,"release must not commit");
    check(bool(m.relocate(p.moves))&&m.nodes().at(10).position.rotated,"ghost differs from committed orientation");
    std::cout<<"PASS rotation release and exact proposal\n";
    check(bool(drag.begin(m,10))&&drag.rotate(m),"begin second");
    check(drag.preview(m,2,0,0,false,{11}).feedback==DropFeedback::Invalid,"disabled swap");
    check(drag.preview(m,2,0,0,true,{}).result.error=="swap_target_unavailable","hidden target swap");
    // The outgoing 2x1 item cannot fit in the source's rightmost column.
    check(drag.preview(m,2,0,0,true,{11}).feedback==DropFeedback::Invalid,"invalid outgoing fit");
    check(bool(m.move({10,1,{0,1,false},m.revision(),m.session()})),"reset placement");
    check(drag.preview(m,2,0,0,true,{11}).result.error=="stale_drag","stale gesture");
    check(bool(drag.begin(m,10)),"new drag"); p=drag.release(m,2,0,0,true,{11});
    check(p.feedback==DropFeedback::Swap&&p.moves.size()==2,"orange swap");
    check(bool(m.relocate(p.moves))&&m.nodes().at(10).parent==2&&m.nodes().at(11).parent==1,"atomic swap commit");
    std::cout<<"PASS swap toggle permissions outgoing fit and atomic commit\n";
    check(bool(drag.begin(m,10)),"begin reload"); const auto saved=m.serialize(); check(bool(m.deserialize(saved)),"reload");
    check(!drag.rotate(m)&&!drag.item(),"reload invalidates drag"); check(bool(drag.begin(m,10)),"begin cancel");
    drag.cancel(); check(drag.preview(m,1,0,0,true,{11}).feedback==DropFeedback::None,"cancel ghost");
    check(m.serialize()==saved,"cancel mutation");
    std::cout<<"PASS session reset and cancel\n";
    check(bool(drag.begin(m,10)),"extreme begin");
    check(drag.preview(m,1,2147483647,0,true,{11}).result.error=="position","extreme coordinate");
    std::cout<<"PASS bounded pointer coordinates\n";
    return 0;
 } catch(const std::exception& e) { std::cerr<<"FAIL "<<e.what()<<'\n'; return 1; }
}
