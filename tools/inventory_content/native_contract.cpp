#include "model.h"
#include <windows.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace inventory_next;
struct Fixture { std::string name; Size grid; std::vector<unsigned> cells;
    std::map<unsigned,std::string> names; unsigned capacity; };
#include "fixtures.inc"

void require(bool ok, const std::string& message) { if(!ok) throw std::runtime_error(message); }
Node root(Id id, Kind kind, Size grid) {
    Node n; n.id=id; n.kind=kind; n.category=0; n.container=true; n.grid=grid; return n;
}
Node target(const Fixture& f) {
    auto n=root(1,Kind::ActorRoot,f.grid); n.cells=f.cells; n.compartment_names=f.names; return n;
}
unsigned checks=0;
void pass(const Fixture& f, const std::string& name) {
    ++checks; std::cout<<"PASS\t"<<f.name<<'\t'<<name<<'\n';
}
void placement(const Fixture& f, int x,int y,int w,int h,bool rotate,const std::string& expected,const std::string& label) {
    Model m; Node probe; probe.id=3; probe.parent=2; probe.empty={w,h};
    auto initialized=m.initialize({target(f),root(2,Kind::WorldRoot,{64,64}),probe});
    require(initialized.ok,"initialize: "+initialized.error);
    auto before=m.serialize();
    Command cmd; cmd.operation=Operation::Move; cmd.item=3; cmd.destination=1;
    cmd.position={x,y,rotate}; cmd.revision=m.revision(); cmd.session=m.session();
    auto preview=m.preview(cmd);
    require(m.serialize()==before,"preview mutated state");
    auto result=m.execute(cmd);
    require(result.ok==expected.empty(),label+": unexpected success/failure "+result.error);
    require(preview.result.ok==result.ok && preview.result.error==result.error,"preview/commit mismatch");
    if(!expected.empty()) {
        require(result.error==expected,label+": expected "+expected+", got "+result.error);
        require(m.serialize()==before,"rejection mutated state");
    } else require(m.nodes().at(3).parent==1,"accepted ownership not applied");
    pass(f,label);
}
int main() {
    try {
        require(GetConsoleWindow()==nullptr,"console attached");
        require(GetPriorityClass(GetCurrentProcess())==IDLE_PRIORITY_CLASS,"not idle priority");
        std::cout<<"ENV\thidden_no_console\tidle_priority\n";
        for(const auto& f:fixtures) {
            unsigned usable=0;
            std::vector<Node> full{target(f),root(2,Kind::WorldRoot,{64,64})};
            Id next=3;
            for(int y=0;y<f.grid.h;++y) for(int x=0;x<f.grid.w;++x) {
                unsigned region=f.cells[y*f.grid.w+x];
                std::string xy=std::to_string(x)+","+std::to_string(y);
                placement(f,x,y,1,1,false,region?"":"blocked_cell",(region?"cell_fit:":"blocked:")+xy);
                if(region) { ++usable; Node n; n.id=next++; n.parent=1; n.position={x,y,false}; n.intrinsic=1; full.push_back(n); }
                if(region && x+1<f.grid.w && f.cells[y*f.grid.w+x+1] && region!=f.cells[y*f.grid.w+x+1])
                    placement(f,x,y,2,1,false,"compartment","cross_horizontal:"+xy);
                if(region && y+1<f.grid.h && f.cells[(y+1)*f.grid.w+x] && region!=f.cells[(y+1)*f.grid.w+x])
                    placement(f,x,y,1,2,false,"compartment","cross_vertical:"+xy);
            }
            require(usable==f.capacity,"normalized capacity differs from cells");
            Model filled; auto initialized=filled.initialize(full); require(initialized.ok,"fill: "+initialized.error);
            require(filled.carried_mass()==usable,"full capacity did not map to owned cells");
            for(const auto& [region,name]:f.names) {
                int minx=f.grid.w,miny=f.grid.h,maxx=-1,maxy=-1;
                for(int y=0;y<f.grid.h;++y) for(int x=0;x<f.grid.w;++x) if(f.cells[y*f.grid.w+x]==region) {
                    minx=std::min(minx,x); miny=std::min(miny,y); maxx=std::max(maxx,x); maxy=std::max(maxy,y);
                }
                require(maxx>=0,"missing named region");
                placement(f,minx,miny,maxx-minx+1,maxy-miny+1,false,"","compartment_fit:"+name);
                placement(f,minx,miny,maxy-miny+1,maxx-minx+1,true,"","rotated_fit:"+name);
            }
            auto before=filled.serialize();
            Command extra; extra.operation=Operation::Spawn; extra.revision=filled.revision(); extra.session=filled.session();
            extra.spawned.id=next; extra.spawned.parent=1;
            auto first=full.at(2).position; extra.spawned.position=first;
            auto rejected=filled.execute(extra);
            require(!rejected.ok && rejected.error=="overlap","full grid accepted an extra item");
            require(filled.serialize()==before,"full-capacity rejection mutated state");
            Model loaded; auto restored=loaded.deserialize(before); require(restored.ok,"reload: "+restored.error);
            require(loaded.nodes().at(1).cells==f.cells && loaded.nodes().at(1).compartment_names==f.names,"region persistence mismatch");
            require(loaded.carried_mass()==usable,"capacity reload mismatch");
            pass(f,"capacity_fill_reject_and_reload");
        }
        std::cout<<"TOTAL\t"<<checks<<'\n'; return 0;
    } catch(const std::exception& e) { std::cerr<<"FAIL\t"<<e.what()<<'\n'; return 1; }
}
