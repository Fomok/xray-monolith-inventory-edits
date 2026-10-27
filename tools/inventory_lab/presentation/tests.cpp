#include "presentation.h"
#include <iostream>
#include <stdexcept>
#include <limits>
using namespace inventory_next;
void check(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
Node root(Id id, Kind kind) { Node n; n.id=id; n.kind=kind; n.category=0; n.container=true; n.grid={8,12}; return n; }
Node item(Id id, Id parent, int x=0, int y=0) { Node n; n.id=id; n.parent=parent; n.position={x,y,false}; n.intrinsic=10; return n; }
Model fixture() {
    auto actor=root(1,Kind::ActorRoot); actor.allowed_slots=3; actor.equipment_accepts=1; actor.equipment_capacity=2;
    auto bag=item(10,1); bag.container=true; bag.grid={4,12};
    auto a=item(30,1), b=item(31,1); a.location=b.location=Location::Equipment;
    a.wear_slots=b.wear_slots=3; a.equipped_slots=1; b.equipped_slots=2; a.presentation_group=b.presentation_group=7;
    auto worldbag=item(20,2); worldbag.container=true; worldbag.grid={4,4};
    auto tall=item(12,10,0,3); tall.empty=tall.half=tall.full={1,3};
    Model m; const auto r=m.initialize({actor,root(2,Kind::WorldRoot),bag,item(11,10),tall,item(13,10,0,10),worldbag,item(21,20),item(22,2,2),a,b});
    if(!r) throw std::runtime_error(r.error);
    return m;
}
int main() {
    try {
        auto m=fixture(); const auto original=m.serialize(); Presentation p;
        check(!p.open(m,1),"unsynchronized open"); p.synchronize(m,1,{});
        check(p.open(m,1)&&p.open(m,10,4)&&p.open(m,10),"carried panels");
        check(p.panels(m).size()==2,"duplicate panel");
        check(!p.open(m,11)&&!p.open(m,999)&&!p.open(m,2),"invalid/unobserved container");
        check(!p.viewport(m,10,0)&&!p.viewport(m,10,65),"viewport bounds");
        check(p.visible_items(m,1).size()==1,"equipped items in grid");
        std::cout<<"PASS panel admission and equipment exclusion\n";
        check(p.visible_items(m,10).size()==2,"initial viewport");
        check(p.scroll(m,10,4),"scroll");
        auto visible=p.visible_items(m,10); check(visible.size()==1&&visible[0].id==12,"partial tile intersection");
        p.scroll(m,10,std::numeric_limits<int>::max()); check(p.panels(m)[1].first_row==8,"scroll upper clamp");
        check(p.visible_items(m,10).at(0).id==13,"bottom item");
        p.scroll(m,10,std::numeric_limits<int>::min()); check(p.panels(m)[1].first_row==0,"scroll lower clamp");
        p.collapse(10,true); check(p.visible_items(m,10).empty(),"collapsed content"); p.collapse(10,false);
        p.viewport(m,10,12); check(p.visible_items(m,10).size()==3,"expanded viewport");
        std::cout<<"PASS independent scrolling collapse and clipping\n";
        p.synchronize(m,1,{2,20}); check(p.open(m,2)&&p.open(m,20),"observed world panels");
        check(p.visible_items(m,20).empty(),"hidden world item leaked");
        p.synchronize(m,1,{2,20,21}); check(p.visible_items(m,20).size()==1,"revealed world item");
        check(p.visible_items(m,2).size()==1,"unrevealed floor item leaked");
        p.synchronize(m,1,{}); check(p.panels(m).size()==2,"lost access did not close world panels");
        std::cout<<"PASS world permissions and per-item reveal\n";
        check(p.cycle(m,7,1)==30&&p.cycle(m,7,1)==31&&p.cycle(m,7,1)==30,"equipment forward cycle");
        check(p.cycle(m,7,-1)==31&&p.cycle(m,7,0)==31&&p.cycle(m,8,1)==0,"equipment reverse cycle");
        check(m.serialize()==original,"presentation changed inventory");
        std::cout<<"PASS stable equipment selection and zero model mutation\n";
        Command erase; erase.operation=Operation::Erase; erase.item=31; erase.session=m.session(); erase.revision=m.revision();
        check(bool(m.execute(erase)),"erase fixture"); check(p.panels(m).empty()&&!p.scroll(m,10,1)&&p.cycle(m,7,1)==0,"stale projection");
        p.synchronize(m,1,{}); check(p.selected(7)==30,"removed selection fallback");
        check(p.panels(m).size()==2,"normal update closed valid panels");
        std::cout<<"PASS stale revision and removed equipment fallback\n";
        const auto saved=m.serialize(); check(bool(m.deserialize(saved)),"reload fixture"); p.synchronize(m,1,{});
        check(p.panels(m).empty()&&p.selected(7)==0,"new session kept old view bindings");
        check(p.open(m,10)&&p.close(10)&&!p.close(10),"close semantics");
        check(m.serialize()==saved,"reload view mutated model");
        std::cout<<"PASS session reset and close invariants\n";
        p.open(m,10); check(bool(m.move({10,2,{4,4,false},m.revision(),m.session()})),"drop bag");
        p.synchronize(m,1,{}); check(p.panels(m).empty(),"dropped inaccessible bag kept open");
        std::cout<<"PASS dropped-container access invalidation\n";
        std::cout<<"7 presentation scenarios passed\n"; return 0;
    } catch(const std::exception& e) { std::cerr<<"FAIL "<<e.what()<<'\n'; return 1; }
}
