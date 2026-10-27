#include "placement.h"
#include <iostream>
#include <stdexcept>
using namespace inventory_next;
void check(bool ok,const char* message) { if(!ok) throw std::runtime_error(message); }
Node root(Id id,Kind kind,Size size) { Node n; n.id=id; n.kind=kind; n.category=0; n.container=true; n.grid=size; return n; }
Node item(Id id,Id parent,Size size,Position pos) { Node n; n.id=id; n.parent=parent; n.empty=n.half=n.full=size; n.position=pos; n.intrinsic=10; return n; }
int main() {
    try {
        Model m;
        auto actor=root(1,Kind::ActorRoot,{3,3}), stash=root(2,Kind::WorldRoot,{4,6});
        auto full=item(10,1,{3,3},{});
        auto a=item(11,2,{2,1},{2,5,false}), b=item(12,2,{2,2},{0,3,false});
        auto ok=m.initialize({actor,stash,full,a,b}); check(bool(ok),ok.error.c_str());
        const auto before=m.serialize();
        check(!plan_stash_sort(m,1,true,{}).result&&!plan_stash_sort(m,2,false,{}).result,"player sort permitted");
        auto plan=plan_stash_sort(m,2,true,{{11,0},{12,1}});
        check(bool(plan.result)&&plan.moves.size()==2&&m.serialize()==before,"sort preview mutated inventory");
        check(bool(m.relocate(plan.moves)),"sort commit");
        check(m.nodes().at(11).position.y==0&&m.nodes().at(12).position.y==0,"sort not compact");
        check(m.carried_mass()==10&&m.subtree_mass(2)==20,"sort changed mass");
        check(!m.relocate(plan.moves),"stale plan accepted");
        std::cout<<"PASS stash-only compact atomic sort and stale guard\n";
        plan=find_placement(m,11,{1,2}); check(bool(plan.result)&&plan.moves.at(0).destination==2,"ordered full-target fallback");
        const auto stable=m.serialize(); check(!find_placement(m,11,{1},true,1).result,"failed admission");
        check(m.serialize()==stable,"placement planning changed source");
        std::cout<<"PASS ordered quick-store fallback and no mutation\n";
        Model rotated;
        ok=rotated.initialize({root(1,Kind::ActorRoot,{1,3}),root(2,Kind::WorldRoot,{3,3}),item(10,2,{3,1},{})});
        check(bool(ok),ok.error.c_str());
        check(!find_placement(rotated,10,{1},false).result,"rotation ignored");
        auto fit=find_placement(rotated,10,{1}); check(bool(fit.result)&&fit.moves.at(0).position.rotated,"rotation-only fit");
        check(bool(rotated.relocate(fit.moves)),"rotation commit");
        std::cout<<"PASS rotation-only first fit\n";
        Model masked; auto shaped=root(2,Kind::WorldRoot,{4,2}); shaped.cells={1,1,0,2,1,1,0,2};
        ok=masked.initialize({root(1,Kind::ActorRoot,{4,4}),shaped,item(10,2,{2,1},{0,1,false}),item(11,2,{1,2},{3,0,false})});
        check(bool(ok),ok.error.c_str()); auto maskedplan=plan_stash_sort(masked,2,true,{});
        check(bool(maskedplan.result),"masked sort");
        if(!maskedplan.moves.empty()) check(bool(masked.relocate(maskedplan.moves)),"masked candidate invalid");
        check(masked.nodes().at(11).position.x==3,"crossed compartments");
        std::cout<<"PASS blocked cells and stable compartment boundaries\n";
        auto first=plan_stash_sort(masked,2,true,{}),second=plan_stash_sort(masked,2,true,{});
        check(first.moves.size()==second.moves.size()&&first.probes==second.probes,"nondeterministic planning");
        check(!find_placement(masked,999,{2}).result&&!find_placement(masked,10,{2},true,0).result,"invalid search");
        std::cout<<"PASS deterministic bounded searches\n";
        return 0;
    } catch(const std::exception& e) { std::cerr<<"FAIL "<<e.what()<<'\n'; return 1; }
}
