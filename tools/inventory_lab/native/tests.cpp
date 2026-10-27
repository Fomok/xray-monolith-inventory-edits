#include "model.h"
#include "persistence.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <limits>
#include <sstream>
#include <iomanip>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
using namespace inventory_next;
namespace {
void check(bool value, const char* description) { if (!value) throw std::runtime_error(description); }
void yes(Result r) { if (!r) throw std::runtime_error("unexpected failure: " + r.error); }
void no(Result r, const char* why) { check(!r && r.error == why, ("expected " + std::string(why) + ", got " + r.error).c_str()); }
Node root(Id id, Kind kind, Size grid = {16,16}) {
    Node n; n.id=id; n.kind=kind; n.category=0; n.container=true; n.grid=grid; return n;
}
Node item(Id id, Id parent, Mass mass, Position p = {}) {
    Node n; n.id=id; n.parent=parent; n.intrinsic=mass; n.position=p; return n;
}
Node bag(Id id, Id parent, Mass mass, Position p = {}) {
    Node n=item(id,parent,mass,p); n.container=true; n.soft=true;
    n.grid={2,2}; n.half={2,1}; n.full={2,2}; return n;
}
std::vector<Node> fixture() {
    return {root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(10,1,100),
            bag(11,10,20),item(12,11,30),item(13,1,7,{0,1,false}),
            item(20,2,40),item(21,2,50,{2,0,false})};
}
Model make() { Model m; yes(m.initialize(fixture())); return m; }
Result move(Model& m, Move request) { request.session=m.session(); return m.move(request); }
void rollback(Model& m, Move request, const char* why) {
    auto before=m.serialize(); no(move(m,request),why); check(m.serialize()==before,"failed transaction changed state");
}
// Test helper for valid-checksum malformed schema/ownership payloads.
std::string reseal(std::string body) {
    std::uint32_t crc=~std::uint32_t{0};
    for (unsigned char c:body) { crc^=c; for(int i=0;i<8;++i) crc=(crc>>1)^(0xedb88320U & (0U-(crc&1U))); }
    std::ostringstream s; s<<body<<"CRC32 "<<std::hex<<std::setw(8)<<std::setfill('0')<<~crc<<'\n'; return s.str();
}
}
int main(int argc,char** argv) {
    try {
        if(argc==3 && std::string(argv[1])=="--read") {
            Model m; yes(load_file(m,argv[2])); check(m.carried_mass()==197,"restart mass");
            check(m.revision()==1 && m.nodes().at(20).parent==11,"restart ownership/revision");
            std::cout<<"PASS independent process persistence\n"; return 0;
        }
        if(argc!=2) throw std::runtime_error("usage: tests.exe fixture.inv");
        int passed=0;
        auto test=[&](const char* name, const std::function<void()>& fn) {
            fn(); ++passed; std::cout<<"PASS "<<name<<'\n';
        };
        test("recursive mass counts every unique nested item once",[]{
            auto m=make(); check(m.carried_mass()==157,"carried sum");
            check(m.subtree_mass(10)==150,"outer contents");
            check(m.subtree_mass(11)==50,"inner contents");
        });
        test("world items visible before pickup; no mass or ownership mutation",[]{
            auto m=make(); auto before=m.serialize();
            check(m.world_items()==std::vector<Id>({20,21}),"world projection");
            check(m.serialize()==before,"preview mutation");
        });
        test("pickup updates ownership and exact recursive mass; drop reverses",[]{
            auto m=make(); yes(move(m,{20,11,{1,0,false},0})); check(m.carried_mass()==197,"pickup weight");
            check(m.world_items()==std::vector<Id>({21}),"pickup world state");
            yes(move(m,{20,2,{0,0,false},1})); check(m.carried_mass()==157,"drop weight");
        });
        test("nested root transfer moves complete tree once",[]{
            auto m=make(); yes(move(m,{10,2,{4,4,false},0})); check(m.carried_mass()==7,"drop bag");
            yes(move(m,{10,1,{0,0,false},1})); check(m.carried_mass()==157,"pickup bag");
        });
        test("grandparent growth collision rolls back every field",[]{
            auto m=make(); yes(move(m,{20,11,{1,0,false},0}));
            rollback(m,{21,11,{0,1,false},1},"overlap");
            check(m.footprint(10).h==1 && m.footprint(11).h==1,"stale growth");
            yes(move(m,{13,1,{4,4,false},1})); yes(move(m,{21,11,{0,1,false},2}));
            check(m.footprint(10).h==2 && m.footprint(11).h==2,"recursive growth missing");
            check(m.carried_mass()==247,"grown mass");
        });
        test("soft bag shrinks after removing final contents",[]{
            auto m=make(); yes(move(m,{12,2,{5,0,false},0}));
            check(m.footprint(11).w==1,"empty inner");
            yes(move(m,{11,2,{7,0,false},1})); check(m.footprint(10).w==1,"empty outer");
        });
        test("growth beyond ancestor grid boundary rolls back pickup",[]{
            auto v=fixture(); v[0].grid={2,1}; v.erase(v.begin()+5);
            Model m; yes(m.initialize(v)); yes(move(m,{20,11,{1,0,false},0}));
            rollback(m,{21,11,{0,1,false},1},"bounds");
        });
        test("cycle and self-parent rejected atomically",[]{
            auto m=make(); rollback(m,{10,11,{0,0,false},0},"cycle");
            rollback(m,{11,11,{0,0,false},0},"cycle");
        });
        test("rigid restricted box remains fixed and rejects wrong category",[]{
            auto v=fixture(); v[2].soft=false; v[2].empty={2,2}; v[5].position={4,4,false};
            v[3].soft=false; v[3].empty={2,2}; v[3].accepts=1; v[7].category=2;
            Model m; yes(m.initialize(v)); yes(move(m,{20,11,{1,0,false},0}));
            check(m.footprint(10).w==2 && m.footprint(11).h==2,"rigid resized");
            rollback(m,{21,11,{0,1,false},1},"restriction");
        });
        test("rotation changes spatial fit and persists in model",[]{
            auto a=root(1,Kind::ActorRoot,{1,2}); auto w=root(2,Kind::WorldRoot);
            auto i=item(3,2,20); i.empty={2,1}; Model m; yes(m.initialize({a,w,i}));
            rollback(m,{3,1,{0,0,false},0},"bounds"); yes(move(m,{3,1,{0,0,true},0}));
            Model n; yes(n.deserialize(m.serialize())); check(n.nodes().at(3).position.rotated,"rotation save");
        });
        test("ancestor recursive mass limit blocks child insertion",[]{
            auto v=fixture(); v[2].max_contents=60; Model m; yes(m.initialize(v));
            rollback(m,{20,11,{1,0,false},0},"mass_capacity");
        });
        test("actor carried limit includes nested children",[]{
            auto v=fixture(); v[0].max_contents=160; Model m; yes(m.initialize(v));
            rollback(m,{20,11,{1,0,false},0},"mass_capacity");
        });
        test("stale UI request cannot overwrite newer placement",[]{
            auto m=make(); yes(move(m,{20,11,{1,0,false},0})); rollback(m,{21,1,{5,5,false},0},"stale_revision");
        });
        test("missing destination and root moves rejected",[]{
            auto m=make(); rollback(m,{20,999,{0,0,false},0},"parent"); rollback(m,{1,2,{},0},"item");
        });
        test("invalid definitions orphan duplicate and disconnected cycle rejected",[]{
            auto m=make(); auto old=m.serialize(); auto v=fixture(); v.push_back(v[2]);
            no(m.initialize(v),"duplicate_id"); check(m.serialize()==old,"duplicate replaced live model");
            v=fixture(); v[2].parent=999; no(m.initialize(v),"parent");
            v=fixture(); v[2].parent=11; no(m.initialize(v),"cycle");
            v=fixture(); v[2].full={1,1}; no(m.initialize(v),"growth_definition");
            v=fixture(); v[2].intrinsic=-1; no(m.initialize(v),"identity_or_mass");
        });
        test("mass overflow and excessive depth rejected",[]{
            Model m; auto a=root(1,Kind::ActorRoot); auto i=item(2,1,1000000000000LL);
            auto j=item(3,1,1,{2,0,false}); no(m.initialize({a,i,j}),"mass_overflow");
            std::vector<Node> v{a}; for(Id id=2;id<68;++id) {auto b=bag(id,id-1,1); b.soft=false; v.push_back(b);}
            no(m.initialize(v),"depth");
        });
        test("canonical save roundtrip including revision definitions and ownership",[]{
            auto m=make(); yes(move(m,{20,11,{1,0,false},0})); auto bytes=m.serialize();
            Model n; yes(n.deserialize(bytes)); check(n.serialize()==bytes,"roundtrip differs");
        });
        test("truncated corrupt and oversized save preserve live model",[]{
            auto m=make(); auto old=m.serialize();
            check(!m.deserialize(old.substr(0,old.size()/2)),"truncation accepted");
            auto corrupt=old; corrupt[30]^=1; check(!m.deserialize(corrupt),"corrupt accepted");
            check(!m.deserialize(std::string(2*1024*1024+1,'x')),"oversized accepted");
            check(m.serialize()==old,"load failure changed model");
        });
        test("valid-checksum unsupported schema and duplicate IDs rejected",[]{
            auto m=make(); auto old=m.serialize(); auto body=old.substr(0,old.rfind("CRC32 "));
            auto changed=body; changed.replace(15,1,"9"); no(m.deserialize(reseal(changed)),"header");
            auto start=body.find("20 2 0 "); check(start!=std::string::npos,"test fixture record");
            body.replace(start,2,"21"); no(m.deserialize(reseal(body)),"duplicate_id");
            check(m.serialize()==old,"invalid load changed model");
        });
        test("repeated pickup/drop conserves total mass and occupancy",[]{
            auto m=make(); for(int i=0;i<300;++i) {
                yes(move(m,{20,11,{1,0,false},m.revision()})); check(m.carried_mass()==197,"pickup conservation");
                yes(move(m,{20,2,{0,0,false},m.revision()})); check(m.carried_mass()==157,"drop conservation");
            }
        });
        test("spawn preview is read-only and commit has identical predicted mass",[]{
            auto m=make(); Command c; c.operation=Operation::Spawn;c.spawned=item(30,11,40,{1,0,false});c.session=m.session();
            auto before=m.serialize();auto p=m.preview(c);yes(p.result);
            check(m.serialize()==before&&m.high_water_id()==21,"preview mutated model");
            yes(m.execute(c));check(m.carried_mass()==p.carried_mass&&m.nodes().size()==p.node_count,"preview parity");
        });
        test("failed spawn through ancestor leaves identity and state intact",[]{
            auto m=make();yes(move(m,{20,11,{1,0,false},0}));Command c;c.operation=Operation::Spawn;
            c.spawned=item(30,11,40,{0,1,false});c.revision=1;c.session=m.session();auto before=m.serialize();
            no(m.execute(c),"overlap");check(m.serialize()==before&&m.high_water_id()==21,"failed spawn changed identity");
            c.spawned.parent=2;c.spawned.position={5,0,false};yes(m.execute(c));
        });
        test("leaf delete updates mass and never reuses committed IDs after reload",[]{
            auto m=make();Command c;c.operation=Operation::Spawn;c.spawned=item(30,2,40,{5,0,false});c.session=m.session();yes(m.execute(c));
            c.operation=Operation::Erase;c.item=30;c.revision=1;yes(m.execute(c));
            auto saved=m.serialize();yes(m.deserialize(saved));c.operation=Operation::Spawn;c.session=m.session();c.revision=2;
            no(m.execute(c),"id_reuse");check(m.serialize()==saved,"reused ID changed state");
            c.operation=Operation::Erase;c.item=12;yes(m.execute(c));check(m.carried_mass()==127,"delete mass");
        });
        test("nonempty containers and roots cannot be erased",[]{
            auto m=make();Command c;c.operation=Operation::Erase;c.item=10;c.session=m.session();auto before=m.serialize();
            no(m.execute(c),"not_empty");c.item=1;no(m.execute(c),"item");check(m.serialize()==before,"delete rollback");
        });
        test("intrinsic mass changes revalidate actor and ancestor caps atomically",[]{
            auto v=fixture();v[0].max_contents=160;Model m;yes(m.initialize(v));Command c;
            c.operation=Operation::SetMass;c.item=12;c.mass=40;c.session=m.session();auto before=m.serialize();
            no(m.execute(c),"mass_capacity");check(m.serialize()==before,"mass rollback");
            c.mass=10;yes(m.execute(c));check(m.carried_mass()==137,"mass delta");
            c.mass=-1;c.revision=1;no(m.execute(c),"identity_or_mass");
        });
        test("preview cannot commit after another command advances revision",[]{
            auto m=make();Command c;c.operation=Operation::SetMass;c.item=12;c.mass=20;c.session=m.session();
            yes(m.preview(c).result);yes(move(m,{13,1,{5,5,false},0}));no(m.execute(c),"stale_revision");
        });
        test("successful reload and reset invalidate sessions even with same revision",[]{
            auto m=make();Command c;c.operation=Operation::SetMass;c.item=12;c.mass=20;c.session=m.session();
            auto before=m.serialize();check(!m.deserialize("broken"),"bad save accepted");
            check(m.session()==c.session,"failed load invalidated epoch");
            yes(m.deserialize(before));check(m.revision()==c.revision,"fixture revision mismatch");
            no(m.execute(c),"stale_session");c.session=m.session();yes(m.initialize(fixture()));no(m.execute(c),"stale_session");
        });
        test("durable file replace then reload and failed write preserves destination",[&]{
            auto m=make(); std::filesystem::path path=argv[1]; yes(save_atomic(m,path));
            yes(move(m,{20,11,{1,0,false},0})); yes(save_atomic(m,path));
            Model n; yes(load_file(n,path)); check(n.serialize()==m.serialize(),"disk roundtrip");
            Model empty; check(!save_atomic(empty,path),"invalid model saved");
            Model again; yes(load_file(again,path)); check(again.serialize()==m.serialize(),"failed save overwrote file");
            check(!save_atomic(m,path/"missing"/"fixture.inv"),"invalid path succeeded");
            HANDLE lock=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
            check(lock!=INVALID_HANDLE_VALUE,"lock fixture");
            yes(move(m,{20,2,{0,0,false},1}));
            auto blocked=save_atomic(m,path);
            CloseHandle(lock);
            no(blocked,"replace");
            Model preserved; yes(load_file(preserved,path));
            check(preserved.serialize()==again.serialize(),"rename failure lost prior save");
        });
        std::cout<<"PASS "<<passed<<" behavioral tests\n"; return 0;
    } catch(const std::exception& e) { std::cerr<<"FAIL "<<e.what()<<'\n'; return 1; }
}
