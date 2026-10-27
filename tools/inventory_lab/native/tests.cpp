#include "../../../src/inventory_next/native_layout.h"
#include "../../../src/inventory_next/native_storage_view.h"
#include "../../../src/inventory_next/storage_catalog.h"
#include "model.h"
#include "access_gate.h"
#include "identity_ledger.h"
#include "identity_state.h"
#include "transfer_session.h"
#include "drag_session.h"
#include "presentation.h"
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
        if(argc==3 && std::string(argv[1])=="--read-logical") {
            Model disk;yes(load_file(disk,argv[2]));ActorLayout restored;yes(restored.restore(disk.serialize()));
            std::vector<Node> facts;
            for(const auto& pair:disk.nodes())if(pair.second.kind==Kind::Item) {
                auto n=pair.second;n.parent=1;n.location=Location::Grid;n.equipped_slots=0;facts.push_back(n);
            }
            yes(restored.reconcile(facts,{{4,4},{},true}));
            check(restored.model().nodes().at(102).parent==101&&restored.model().nodes().at(101).parent==100,"restart flattened nested storage");
            check(restored.model().carried_mass()==1100&&restored.model().subtree_mass(101)==300,"restart recursive mass");
            check(restored.model().nodes().at(100).position.x==2&&restored.model().nodes().at(101).position.y==1,"restart moved containers");
            std::cout<<"PASS independent native logical forest restart\n";return 0;
        }
        if(argc!=2) throw std::runtime_error("usage: tests.exe fixture.inv");
        int passed=0;
        auto test=[&](const char* name, const std::function<void()>& fn) {
            fn(); ++passed; std::cout<<"PASS "<<name<<'\n';
        };
        test("native transfer publishes once only after matching ownership acknowledgment",[]{
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),item(10,2,7)}));
            EntityRegistry registry;for(Id id:{1,2,10}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
            AccessGate access(m);yes(access.observe(2,true,true));yes(access.observe_item(10,true,true));
            Command c;c.operation=Operation::Move;c.item=10;c.destination=1;c.session=m.session();
            TransferSession transfer(m,access,registry);auto old=m.serialize();yes(transfer.prepare({c,access.revision()}));
            check(m.serialize()==old&&!transfer.can_save(),"prepared transfer published early");
            TransferSession::Dispatch event;yes(transfer.dispatch(event));check(m.serialize()==old,"dispatch published early");
            no(transfer.cancel(),"native_reconcile_required");no(transfer.acknowledge(event.token+1,1,true),"transfer_ack");
            yes(transfer.acknowledge(event.token,1,true));check(m.carried_mass()==7&&m.revision()==1&&transfer.can_save(),"ack did not publish exactly once");
            no(transfer.acknowledge(event.token,1,true),"transfer_ack");
        });
        test("one transfer authority serializes two pickups competing for the last cell",[]{
            Model m;yes(m.initialize({root(1,Kind::ActorRoot,{1,1}),root(2,Kind::WorldRoot),item(10,2,7),item(11,2,9,{1,0,false})}));
            EntityRegistry registry;for(Id id:{1,2,10,11}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
            AccessGate access(m);yes(access.observe(2,true,true));yes(access.observe_item(10,true,true));yes(access.observe_item(11,true,true));
            TransferSession transfer(m,access,registry);Command first;first.operation=Operation::Move;first.item=10;first.destination=1;first.session=m.session();
            Command second=first;second.item=11;auto before=m.serialize();
            yes(transfer.prepare({first,access.revision()}));no(transfer.prepare({second,access.revision()}),"transfer_busy");
            check(m.serialize()==before,"competing pickup changed source before ack");
            TransferSession::Dispatch event;yes(transfer.dispatch(event));yes(transfer.acknowledge(event.token,1,true));
            second.revision=m.revision();no(transfer.prepare({second,access.revision()}),"overlap");
            check(m.nodes().at(11).parent==2&&m.nodes().at(11).intrinsic==9&&m.carried_mass()==7&&transfer.can_save(),"losing pickup lost owner or mass");
        });
        test("source disappearance before dispatch cancels and after dispatch requires reconciliation",[]{
            for(bool sent:{false,true}) {
                Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),item(10,2,7)}));
                EntityRegistry registry;for(Id id:{1,2,10}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
                AccessGate access(m);yes(access.observe(2,true,true));yes(access.observe_item(10,true,true));
                TransferSession transfer(m,access,registry);Command command;command.operation=Operation::Move;command.item=10;command.destination=1;command.session=m.session();
                yes(transfer.prepare({command,access.revision()}));TransferSession::Dispatch event;auto before=m.serialize();
                if(sent)yes(transfer.dispatch(event));
                yes(registry.unbind(*registry.handle(2)));
                if(sent){no(transfer.acknowledge(event.token,1,true),"stale_entity");check(!transfer.can_save(),"uncertain source disappearance allowed save");}
                else {no(transfer.dispatch(event),"stale_entity");check(transfer.can_save(),"cancelled preparation held save lock");}
                check(m.serialize()==before&&m.nodes().at(10).parent==2,"source disappearance fabricated transfer");
            }
        });
        test("ground pickup uses no fabricated parent handle and rejection retains ground owner",[]{
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),item(10,2,7)}));
            EntityRegistry registry;for(Id id:{1,10}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
            AccessGate access(m);yes(access.observe(2,true,true));yes(access.observe_item(10,true,true));
            TransferSession transfer(m,access,registry);Command command;command.operation=Operation::Move;command.item=10;command.destination=1;command.session=m.session();
            auto before=m.serialize();no(transfer.prepare({command,access.revision()}),"offline_entity");
            yes(transfer.prepare({command,access.revision()},true));TransferSession::Dispatch event;yes(transfer.dispatch(event));
            check(!event.source,"ground pickup fabricated native source");no(transfer.acknowledge(event.token,0xffff,false),"native_rejected");
            check(m.serialize()==before&&transfer.can_save(),"ground rejection lost item");
            yes(transfer.prepare({command,access.revision()},true));yes(transfer.dispatch(event));yes(transfer.acknowledge(event.token,1,true));
            check(m.nodes().at(10).parent==1&&m.carried_mass()==7,"ground acknowledgment failed");
        });
        test("ground admission cannot disguise an offline source and rechecks reachability",[]{
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),item(10,2,7)}));
            EntityRegistry registry;for(Id id:{1,10}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
            AccessGate access(m);yes(access.observe(2,true,true));yes(access.observe_item(10,true,true));
            TransferSession transfer(m,access,registry);Command command;command.operation=Operation::Move;command.item=10;command.destination=1;command.session=m.session();
            yes(transfer.prepare({command,access.revision()},true));auto before=m.serialize();yes(access.observe(2,true,false));
            TransferSession::Dispatch event;no(transfer.dispatch(event),"stale_access");check(m.serialize()==before&&transfer.can_save(),"unreachable ground pickup published");
            yes(access.observe(2,true,true));yes(registry.add(2));
            no(transfer.prepare({command,access.revision()},true),"native_ground_source");
            check(m.serialize()==before,"offline source spoof changed ownership");
        });
        test("native rejection preserves source and uncertain parent requires explicit rebuild",[]{
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),item(10,1,7)}));
            EntityRegistry registry;for(Id id:{1,2,10}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
            AccessGate access(m);yes(access.observe(2,true,true));Command c;c.operation=Operation::Move;c.item=10;c.destination=2;c.session=m.session();
            TransferSession transfer(m,access,registry);TransferSession::Dispatch event;auto old=m.serialize();
            yes(transfer.prepare({c,access.revision()}));yes(transfer.dispatch(event));no(transfer.acknowledge(event.token,1,false),"native_rejected");
            check(m.serialize()==old&&transfer.can_save(),"rejection lost item");
            yes(transfer.prepare({c,access.revision()}));yes(transfer.dispatch(event));no(transfer.acknowledge(event.token,99,true),"native_parent_mismatch");
            no(transfer.reconciled(),"rebuild_required");check(m.serialize()==old,"uncertain event published");
            yes(m.deserialize(old));yes(transfer.reconciled());check(transfer.can_save(),"explicit recovery failed");
        });
        test("native transfer rejects reused live IDs and concurrent model edits",[]{
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),item(10,1,7)}));
            EntityRegistry registry;for(Id id:{1,2,10}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
            AccessGate access(m);yes(access.observe(2,true,true));Command c;c.operation=Operation::Move;c.item=10;c.destination=2;c.session=m.session();
            TransferSession transfer(m,access,registry);TransferSession::Dispatch event;
            yes(transfer.prepare({c,access.revision()}));yes(registry.unbind(*registry.handle(10)));yes(registry.bind(10,10));
            no(transfer.dispatch(event),"stale_entity");check(transfer.can_save(),"stale preflight held reservation");
            yes(transfer.prepare({c,access.revision()}));yes(transfer.dispatch(event));Command changed;c.revision=m.revision();
            changed.operation=Operation::SetMass;changed.item=10;changed.mass=8;changed.session=m.session();yes(m.execute(changed));
            no(transfer.acknowledge(event.token,2,true),"stale_model");check(m.carried_mass()==8&&!transfer.can_save(),"stale acknowledgment overwrote live state");
        });
        test("dismantling consumes once and retains all outputs with visible remainder",[]{
            Model m;yes(m.initialize({root(1,Kind::ActorRoot,{1,1}),item(10,1,30)}));
            auto a=item(11,1,10);a.attributes={{"part","barrel:0.75"}};auto b=item(12,1,12);b.attributes={{"part","bolt:0.50"}};
            yes(m.recover_recipe({10},{a,b},1,0,m.session()));check(!m.nodes().count(10)&&m.nodes().size()==3&&m.revision()==1,"recipe consumed or published incorrectly");
            check(m.nodes().at(11).location==Location::Grid&&m.nodes().at(12).location==Location::Overflow&&m.carried_mass()==22,"recipe output lost or capacity invented");
            Presentation view;view.synchronize(m,1,{});view.open(m,1);auto layout=view.overflow_layout(m,1);check(layout.tiles.size()==1&&layout.rows==1,"recipe remainder hidden");
            auto saved=m.serialize();Model n;yes(n.deserialize(saved));check(n.serialize()==saved&&n.nodes().at(12).attributes==b.attributes,"recovered part state lost");
        });
        test("invalid dismantling output rolls back input outputs IDs and revision",[]{
            Model m;yes(m.initialize({root(1,Kind::ActorRoot,{1,1}),item(10,1,30)}));auto old=m.serialize();
            auto a=item(11,1,10);auto invalid=item(12,1,-1);
            no(m.recover_recipe({10},{a,invalid},1,0,m.session()),"identity_or_mass");check(m.serialize()==old&&m.high_water_id()==10,"partial recipe escaped");
        });
        test("forced arrival remains visible owned full-size overflow and survives save",[]{
            auto actor=root(1,Kind::ActorRoot,{2,2});actor.allowed_slots=1;actor.equipment_accepts=1;actor.equipment_capacity=1;
            Model m;yes(m.initialize({actor,root(2,Kind::WorldRoot)}));auto rifle=item(10,1,3500);rifle.empty={5,2};rifle.wear_slots=1;rifle.attributes={{"ammo","retained"}};
            Command c;c.operation=Operation::Spawn;c.spawned=rifle;c.session=m.session();no(m.execute(c),"bounds");
            yes(m.recover_native_arrival(rifle,0,m.session()));check(m.carried_mass()==3500&&m.footprint(10).w==5,"overflow lost mass or shrank item");
            Presentation view;view.synchronize(m,1,{});check(view.open(m,1),"panel failed");check(view.visible_items(m,1).empty()&&view.overflow_items(m,1).size()==1,"overflow hidden or legal grid occupied");
            auto display=view.overflow_layout(m,1);check(display.columns==5&&display.rows==2&&m.nodes().at(1).grid.w==2,"overflow display changed legal grid or shrank rifle");
            auto saved=m.serialize();Model restored;yes(restored.deserialize(saved));check(restored.serialize()==saved&&restored.nodes().at(10).location==Location::Overflow,"overflow save lost state");
            rollback(m,{10,1,{0,2,false},1},"bounds");
            c.operation=Operation::Equip;c.item=10;c.destination=1;c.slots=1;c.revision=1;yes(m.execute(c));check(m.carried_mass()==3500&&m.nodes().at(10).attributes.at("ammo")=="retained","overflow equip changed item");
        });
        test("overflow recovery cannot silently pick up world or become manual capacity",[]{
            Model m;yes(m.initialize({root(1,Kind::ActorRoot,{1,1}),root(2,Kind::WorldRoot),item(10,1,5),item(20,2,7)}));
            auto old=m.serialize();no(m.recover_owned_overflow(20,1,0,m.session()),"recovery_ownership");check(m.serialize()==old,"recovery picked up stash/world item");
            yes(m.recover_owned_overflow(10,1,0,m.session()));check(m.carried_mass()==5,"recovery dropped owned item");
            Command c;c.operation=Operation::Spawn;c.spawned=item(30,1,9);c.spawned.location=Location::Overflow;c.session=m.session();c.revision=1;no(m.execute(c),"recovery_only");
            yes(move(m,{10,1,{0,0,false},1}));check(m.nodes().at(10).location==Location::Grid,"overflow could not return to legal space");
        });
        test("forced overweight recovery counts all mass while voluntary pickup remains blocked",[]{
            auto actor=root(1,Kind::ActorRoot,{2,2});actor.max_contents=10;
            Model m;yes(m.initialize({actor,root(2,Kind::WorldRoot),item(10,1,5),item(20,2,1)}));
            yes(m.recover_native_arrival(item(30,1,20),0,m.session()));check(m.carried_mass()==25,"overweight recovery lost mass");
            rollback(m,{20,1,{1,0,false},1},"mass_capacity");yes(move(m,{30,2,{2,0,false},1}));check(m.carried_mass()==5,"overflow drop did not subtract exact mass");
        });
        test("splitting recovered stack creates only legal grid placement",[]{
            Model m;yes(m.initialize({root(1,Kind::ActorRoot,{1,1}),root(2,Kind::WorldRoot),item(10,1,5)}));
            auto ammo=item(20,1,2);ammo.type=1;ammo.stack_limit=10;ammo.quantity=6;yes(m.recover_native_arrival(ammo,0,m.session()));
            Command split;split.operation=Operation::Split;split.item=20;split.new_id=21;split.destination=1;split.quantity=2;split.session=m.session();split.revision=1;
            auto old=m.serialize();no(m.execute(split),"overlap");check(m.serialize()==old,"split manufactured overflow capacity");
            yes(move(m,{10,2,{0,0,false},1}));split.revision=2;yes(m.execute(split));check(m.nodes().at(21).location==Location::Grid&&m.nodes().at(20).location==Location::Overflow&&m.carried_mass()==12,"split lost ownership or location");
        });
        test("native callback remap before delayed destroy invalidates old handles",[]{
            IdentityState state;Id id=0;yes(state.enroll(42,id));yes(state.online(42,1000));auto old=*state.registry().handle(id);
            yes(state.remap(42,84));check(!state.registry().resolve(old),"remap retained stale online handle");
            yes(state.online(84,2000));auto current=*state.registry().handle(id);yes(state.offline(42,1000));
            check(state.registry().resolve(current)==id,"delayed old destroy unbound replacement");
            yes(state.offline(84,1000));check(state.registry().resolve(current)==id,"wrong lifetime unbound live object");
            yes(state.offline(84,2000));check(!state.registry().resolve(current)&&state.ledger().lookup(84)==id,"offline retired persistent identity");
            yes(state.online(84,3000));auto before_load=*state.registry().handle(id);yes(state.restore(state.ledger().serialize()));
            check(!state.registry().resolve(before_load)&&state.ledger().lookup(84)==id,"load kept online handle or lost identity");
        });
        test("native identity survives save restore remap and retires reused server IDs",[]{
            IdentityLedger ledger;Id first=0,again=0;yes(ledger.enroll(42,first));yes(ledger.enroll(42,again));check(first==again,"identity changed");
            const auto saved=ledger.serialize();IdentityLedger restored;yes(restored.deserialize(saved));
            yes(restored.remap(42,84));check(restored.lookup(84)==first&&!restored.lookup(42),"remap changed persistent ID");
            restored.retire(84);yes(restored.enroll(84,again));check(again>first,"destroyed ID reused");
            yes(restored.deserialize(saved));check(restored.lookup(42)==first,"save identity mismatch");
        });
        test("native identity corrupt restore collision and exhaustion fail atomically",[]{
            IdentityLedger ledger;Id id=0;yes(ledger.enroll(1,id));yes(ledger.enroll(2,id));auto old=ledger.serialize();
            no(ledger.remap(1,2),"identity_remap");no(ledger.deserialize("SQA3_IDENTITIES 1 100 2\n1 100\n2 100\n"),"identity_record");
            check(ledger.serialize()==old,"identity failure mutated state");
            no(ledger.enroll(0xffff,id),"invalid_server_id");
            yes(ledger.deserialize("SQA3_IDENTITIES 1 18446744073709551615 0\n"));no(ledger.enroll(3,id),"identity_exhausted");
        });
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
        test("stack split merge conserve quantity mass and stable surviving identity",[]{
            auto a=root(1,Kind::ActorRoot);auto w=root(2,Kind::WorldRoot);auto ammo=item(10,1,5);
            ammo.type=100;ammo.state=7;ammo.quantity=20;ammo.stack_limit=30;Model m;yes(m.initialize({a,w,ammo}));
            Command c;c.operation=Operation::Split;c.item=10;c.new_id=11;c.destination=2;c.quantity=5;c.session=m.session();
            yes(m.execute(c));check(m.nodes().at(10).quantity==15&&m.nodes().at(11).quantity==5,"split identity");
            check(m.carried_mass()==75&&m.subtree_mass(11)==25,"split mass");
            c.operation=Operation::Merge;c.item=11;c.destination=10;c.revision=1;yes(m.execute(c));
            check(m.nodes().count(11)==0&&m.nodes().at(10).quantity==20&&m.carried_mass()==100,"merge conservation");
            Model n;yes(n.deserialize(m.serialize()));check(n.serialize()==m.serialize()&&n.high_water_id()==11,"stack persistence");
        });
        test("partial merge refuses incompatible state and overflow atomically",[]{
            auto a=root(1,Kind::ActorRoot);auto x=item(10,1,5);x.type=10;x.state=1;x.quantity=20;x.stack_limit=30;
            auto y=x;y.id=11;y.position={2,0,false};y.quantity=15;Model m;yes(m.initialize({a,x,y}));
            Command c;c.operation=Operation::Merge;c.item=11;c.destination=10;c.quantity=11;c.session=m.session();auto old=m.serialize();
            no(m.execute(c),"merge_quantity");check(m.serialize()==old,"overflow partial mutation");
            c.quantity=10;yes(m.execute(c));check(m.nodes().at(10).quantity==30&&m.nodes().at(11).quantity==5,"partial quantities");
            y.state=2;yes(m.initialize({a,x,y}));c.session=m.session();c.revision=0;c.quantity=1;
            no(m.execute(c),"stack_compatibility");y.state=1;y.type=99;yes(m.initialize({a,x,y}));c.session=m.session();no(m.execute(c),"stack_compatibility");
        });
        test("split collision rolls back quantities high-water and ancestor growth",[]{
            auto m=make();auto v=fixture();v[6].type=10;v[6].quantity=5;v[6].stack_limit=10;yes(m.initialize(v));
            Command c;c.operation=Operation::Split;c.item=20;c.quantity=2;c.new_id=30;c.destination=11;c.position={0,0,false};c.session=m.session();
            auto old=m.serialize();no(m.execute(c),"overlap");check(m.serialize()==old,"failed split mutation");
            c.position={1,0,false};yes(m.execute(c));check(m.nodes().at(20).quantity==3&&m.nodes().at(30).quantity==2,"successful split");
        });
        test("stack unit-mass update and definition bounds are validated",[]{
            auto a=root(1,Kind::ActorRoot);a.max_contents=110;auto x=item(10,1,5);x.type=10;x.quantity=20;x.stack_limit=30;
            Model m;yes(m.initialize({a,x}));Command c;c.operation=Operation::SetMass;c.item=10;c.mass=6;c.session=m.session();
            no(m.execute(c),"mass_capacity");c.mass=4;yes(m.execute(c));check(m.carried_mass()==80,"unit mass count");
            x.quantity=0;no(m.initialize({a,x}),"stack_definition");x.quantity=31;no(m.initialize({a,x}),"stack_definition");
            x.quantity=2;x.container=true;x.grid={2,2};no(m.initialize({a,x}),"stack_definition");
        });
        test("equipment is outside grid and compatible layers share view group",[]{
            auto a=root(1,Kind::ActorRoot,{1,1});a.allowed_slots=7;a.equipment_accepts=1;a.equipment_capacity=3;
            auto w=root(2,Kind::WorldRoot);auto blocker=item(5,1,1);auto x=item(10,2,10);x.wear_slots=3;x.presentation_group=8;
            auto y=x;y.id=11;y.position={2,0,false};Model m;yes(m.initialize({a,w,blocker,x,y}));
            Command c;c.operation=Operation::Equip;c.item=10;c.destination=1;c.slots=1;c.session=m.session();yes(m.execute(c));
            c.item=11;c.slots=2;c.revision=1;yes(m.execute(c));check(m.carried_mass()==21,"equipment weight");
            check(m.equipment_group(8)==std::vector<Id>({10,11}),"group members");auto old=m.serialize();
            check(m.cycle_equipment(8,10,1)==11&&m.cycle_equipment(8,10,-1)==11,"cycle wrap");
            check(m.cycle_equipment(8,999,1)==10&&m.cycle_equipment(99,10,1)==0,"selection fallback");
            check(m.serialize()==old,"arrows changed equipment");rollback(m,{10,1,{0,0,false},2},"overlap");
            Model n;yes(n.deserialize(old));check(n.nodes().at(10).location==Location::Equipment&&n.equipment_group(8).size()==2,"equipment restore");
        });
        test("equipment slot overlap blocked layers and required slots reject",[]{
            auto a=root(1,Kind::ActorRoot);a.allowed_slots=7;a.equipment_accepts=1;a.equipment_capacity=3;
            auto x=item(10,1,10);x.wear_slots=7;x.blocked_slots=2;auto y=x;y.id=11;y.position={2,0,false};y.blocked_slots=0;
            Model m;yes(m.initialize({a,x,y}));Command c;c.operation=Operation::Equip;c.item=10;c.destination=1;c.slots=1;c.session=m.session();yes(m.execute(c));
            c.item=11;c.revision=1;no(m.execute(c),"equipment_conflict");c.slots=2;no(m.execute(c),"equipment_conflict");
            c.slots=3;no(m.execute(c),"equipment");c.slots=4;yes(m.execute(c));
        });
        test("equipped soft bag grows without grid collision and cannot unequip illegally",[]{
            auto a=root(1,Kind::ActorRoot,{1,1});a.allowed_slots=1;a.equipment_accepts=1;a.equipment_capacity=1;
            auto w=root(2,Kind::WorldRoot);auto b=bag(10,2,100);b.wear_slots=1;auto i=item(11,2,10,{3,0,false});
            Model m;yes(m.initialize({a,w,b,i}));Command c;c.operation=Operation::Equip;c.item=10;c.destination=1;c.slots=1;c.session=m.session();yes(m.execute(c));
            yes(move(m,{11,10,{0,0,false},1}));check(m.footprint(10).w==2&&m.carried_mass()==110,"equipped growth");
            rollback(m,{10,1,{0,0,false},2},"bounds");yes(move(m,{10,2,{0,0,false},2}));check(m.carried_mass()==0,"drop full equipment mass");
        });
        test("structured read snapshot is immutable and reports rotated quantity mass",[]{
            auto a=root(1,Kind::ActorRoot);auto x=item(10,1,5);x.type=10;x.quantity=3;x.stack_limit=5;x.empty={2,1};x.position.rotated=true;
            Model m;yes(m.initialize({a,x}));auto old=m.serialize();auto v=m.snapshot();
            check(v.size()==1&&v[0].quantity==3&&v[0].gross_mass==15&&v[0].footprint.w==1&&v[0].footprint.h==2,"view values");
            check(m.serialize()==old,"snapshot mutation");
        });
        test("world reach changes invalidate preview without inventory revision change",[]{
            auto m=make();AccessGate gate(m);Command c;c.item=20;c.destination=11;c.position={1,0,false};c.session=m.session();
            AccessGate::Ticket t{c,gate.revision()};no(gate.execute(t),"unobserved_source");
            yes(gate.observe(2,true,true));yes(gate.observe_item(20,true,true));t.access_revision=gate.revision();yes(gate.preview(t).result);auto old=m.serialize();
            yes(gate.observe(2,true,false));no(gate.execute(t),"stale_access");check(m.serialize()==old,"reach failure mutation");
            t.access_revision=gate.revision();no(gate.execute(t),"world_unavailable");
            yes(gate.observe(2,true,true));t.access_revision=gate.revision();yes(gate.execute(t));check(m.carried_mass()==197,"guarded transfer");
        });
        test("world and nested views are read-only and observation resets on load",[]{
            auto m=make();AccessGate gate(m);check(gate.children(2).empty(),"unobserved world exposed");
            yes(gate.observe(2,true,true));yes(gate.observe_item(20,true,true));yes(gate.observe_item(21,true,true));auto old=m.serialize();check(gate.children(2)==std::vector<Id>({20,21}),"world direct children");
            check(gate.children(11)==std::vector<Id>({12}),"nested view");check(m.serialize()==old,"view mutation");
            auto rev=gate.revision();yes(m.deserialize(old));check(gate.children(2).empty()&&gate.revision()!=rev,"load retained stale observations");
        });
        test("ordered crafting batch either consumes and creates or changes nothing",[]{
            auto m=make();Command erase;erase.operation=Operation::Erase;erase.item=12;erase.session=m.session();
            Command spawn;spawn.operation=Operation::Spawn;spawn.spawned=item(30,11,60,{2,2,false});spawn.session=m.session();
            auto old=m.serialize();no(m.execute_batch({erase,spawn}),"bounds");check(m.serialize()==old&&m.high_water_id()==21,"craft lost input");
            spawn.spawned.position={0,0,false};yes(m.execute_batch({erase,spawn}));
            check(m.nodes().count(12)==0&&m.nodes().count(30)==1&&m.carried_mass()==187&&m.revision()==1,"craft commit");
        });
        test("batch stale requests and empty batches fail without publishing partial state",[]{
            auto m=make();Command c;c.operation=Operation::SetMass;c.item=12;c.mass=20;c.session=m.session();
            Command stale=c;stale.revision=1;auto old=m.serialize();no(m.execute_batch({c,stale}),"stale_revision");
            check(m.serialize()==old,"batch partial publish");no(m.execute_batch({}),"batch_size");
        });
        test("simultaneous relocation swaps occupied cells and publishes once",[]{
            auto a=root(1,Kind::ActorRoot,{2,1});auto x=item(10,1,10);auto y=item(11,1,20,{1,0,false});Model m;yes(m.initialize({a,x,y}));
            rollback(m,{10,1,{1,0,false},0},"overlap");auto epoch=m.session();
            yes(m.relocate({{10,1,{1,0,false},0,epoch},{11,1,{0,0,false},0,epoch}}));
            check(m.nodes().at(10).position.x==1&&m.carried_mass()==30&&m.revision()==1,"swap commit");
        });
        test("cross-container unequal-size swap validates only complete final arrangement",[]{
            auto a=root(1,Kind::ActorRoot,{2,1});auto w=root(2,Kind::WorldRoot,{2,1});
            auto x=item(10,1,10);x.empty={2,1};auto y=item(11,2,20);Model m;yes(m.initialize({a,w,x,y}));auto e=m.session();
            yes(m.relocate({{10,2,{0,0,false},0,e},{11,1,{0,0,false},0,e}}));check(m.carried_mass()==20,"cross swap mass");
            auto old=m.serialize();no(m.relocate({{10,1,{1,0,false},1,e},{11,2,{0,0,false},1,e}}),"bounds");
            check(m.serialize()==old,"failed final fit mutated model");
        });
        test("layout duplicate targets cycles restrictions and stale sessions reject",[]{
            auto m=make();auto e=m.session();auto old=m.serialize();Move a{20,1,{5,5,false},0,e};
            no(m.relocate({a,a}),"duplicate_target");no(m.relocate({{10,11,{0,0,false},0,e}}),"cycle");
            a.session=0;no(m.relocate({a}),"stale_session");check(m.serialize()==old,"layout rollback");
            auto v=fixture();v[1].accepts=2;v.erase(v.begin()+6,v.end());yes(m.initialize(v));
            no(m.relocate({{12,2,{0,0,false},0,m.session()}}),"restriction");
        });
        test("blocked cells and compartment crossing reject despite sufficient total area",[]{
            auto a=root(1,Kind::ActorRoot,{2,2});a.cells={1,2,1,2};auto w=root(2,Kind::WorldRoot);auto x=item(10,2,10);x.empty={2,1};
            Model m;yes(m.initialize({a,w,x}));rollback(m,{10,1,{0,0,false},0},"compartment");
            yes(move(m,{10,1,{0,0,true},0}));check(m.cell_region(1,1,0)==2,"region projection");
            a.cells={1,0,1,0};yes(m.initialize({a,w,x}));rollback(m,{10,1,{0,0,false},0},"blocked_cell");
            check(m.cell_region(1,1,0)==0&&m.cell_region(1,-1,0)==0,"blocked projection");
        });
        test("soft fullness uses usable cells and region data survives saving",[]{
            auto a=root(1,Kind::ActorRoot);auto b=bag(10,1,100);b.cells={1,0,1,0};auto x=item(11,10,10);
            auto w=root(2,Kind::WorldRoot);auto y=item(12,2,10);Model m;yes(m.initialize({a,w,b,x,y}));
            check(m.footprint(10).h==1,"half usable capacity");yes(move(m,{12,10,{0,1,false},0}));
            check(m.footprint(10).h==2,"full usable capacity");Model n;yes(n.deserialize(m.serialize()));
            check(n.nodes().at(10).cells==b.cells&&n.serialize()==m.serialize(),"mask persistence");
        });
        test("invalid masks cannot create unreachable legal capacity",[]{
            Model m;auto a=root(1,Kind::ActorRoot,{2,2});a.cells={1};no(m.initialize({a}),"cell_definition");
            a.cells={0,0,0,0};no(m.initialize({a}),"cell_definition");a.cells={1,1,4097,1};no(m.initialize({a}),"cell_definition");
        });
        test("world action admission rejects hidden children and non-actionable ancestors",[]{
            auto a=root(1,Kind::ActorRoot);auto w=root(2,Kind::WorldRoot);auto b=bag(10,2,100);auto x=item(11,10,20);
            Model m;yes(m.initialize({a,w,b,x}));AccessGate gate(m);yes(gate.observe(2,true,true));yes(gate.observe_item(10,true,true));
            check(gate.children(10).empty(),"hidden child disclosed");Command c;c.item=11;c.destination=1;c.session=m.session();
            AccessGate::Ticket t{c,gate.revision()};no(gate.execute(t),"unrevealed_item");
            yes(gate.observe_item(11,true,true));yes(gate.observe_item(10,true,false));t.access_revision=gate.revision();
            check(gate.children(10)==std::vector<Id>({11}),"visible child withheld");no(gate.execute(t),"item_action_denied");
            yes(gate.observe_item(10,true,true));t.access_revision=gate.revision();yes(gate.execute(t));check(m.carried_mass()==20,"revealed pickup");
        });
        test("mounted pouch identity remains stable when first neighboring pouch is removed",[]{
            auto a=root(1,Kind::ActorRoot);auto w=root(2,Kind::WorldRoot);auto rig=item(10,1,100);rig.container=true;rig.grid={2,2};
            rig.mounts={{101,"b1",{1,1},2,5000,true},{102,"b2",{1,1},2,5000,true}};
            auto p=item(20,1,20,{3,0,false});p.category=2;p.container=true;p.pouch=true;p.grid={1,2};p.compartment_names={{1,"p1_1"}};
            auto q=p;q.id=21;q.position={4,0,false};auto x=item(30,2,25);Model m;yes(m.initialize({a,w,rig,p,q,x}));
            Command c;c.operation=Operation::Mount;c.item=20;c.destination=10;c.mount_slot=101;c.session=m.session();yes(m.execute(c));
            c.item=21;c.mount_slot=102;c.revision=1;yes(m.execute(c));yes(move(m,{30,21,{0,0,false},2}));
            yes(move(m,{20,1,{3,0,false},3}));check(m.nodes().at(21).mounted_slot==102&&m.nodes().at(30).parent==21,"renumbered pouch storage");
            rollback(m,{21,1,{4,0,false},4},"mounted_not_empty");check(m.carried_mass()==165,"mounted mass counted twice");
            auto saved=m.serialize();Model n;yes(n.deserialize(saved));check(n.serialize()==saved&&n.nodes().at(21).compartment_names.at(1)=="p1_1","named pouch restore");
        });
        test("mounted contents grow soft rig and reject grandparent overlap atomically",[]{
            auto a=root(1,Kind::ActorRoot);auto w=root(2,Kind::WorldRoot);
            auto outer=bag(9,1,5);outer.grid={3,1};outer.half={2,1};outer.full={4,2};
            auto rig=bag(10,9,100);rig.grid={1,1};rig.half={2,1};rig.full={3,1};
            rig.mounts={{101,"b1",{1,1},2,0,true}};
            auto p=item(20,10,20);p.category=2;p.container=true;p.pouch=true;p.grid={1,2};p.location=Location::Mounted;p.mounted_slot=101;
            auto blocker=item(40,1,1,{2,0,false});auto loot=item(30,2,7);
            Model m;yes(m.initialize({a,w,outer,rig,p,blocker,loot}));
            check(m.footprint(10).w==1,"empty mounted pouch expanded rig");
            rollback(m,{30,20,{0,0,false},0},"overlap");
            yes(move(m,{40,1,{8,0,false},0}));yes(move(m,{30,20,{0,0,false},1}));
            check(m.footprint(10).w==2&&m.footprint(9).w==4,"mounted contents failed ancestor growth");
            check(m.carried_mass()==133,"pouch contents double counted");
            yes(move(m,{30,2,{0,0,false},2}));check(m.footprint(10).w==1,"unloaded rig did not fold");
        });
        test("shaped soft storage excludes blocked gutters from growth capacity",[]{
            auto a=root(1,Kind::ActorRoot);auto b=bag(10,1,5);b.grid={3,1};b.cells={1,0,2};b.half={1,1};b.full={2,1};
            Model m;yes(m.initialize({a,b,item(20,10,2),item(21,10,3,{2,0,false})}));
            check(m.footprint(10).w==2,"usable-cell growth threshold");
        });
        test("mount restrictions condition and occupied slot failures preserve original pouch",[]{
            auto a=root(1,Kind::ActorRoot);auto rig=item(10,1,100);rig.container=true;rig.grid={1,1};rig.mounts={{1,"b1",{1,1},2,5000,true}};
            auto p=item(20,1,20,{3,0,false});p.category=2;p.container=true;p.pouch=true;p.grid={1,2};auto q=p;q.id=21;q.position={4,0,false};
            Model m;yes(m.initialize({a,rig,p,q}));Command c;c.operation=Operation::Mount;c.item=20;c.destination=10;c.mount_slot=1;c.session=m.session();yes(m.execute(c));
            c.item=21;c.revision=1;auto old=m.serialize();no(m.execute(c),"mount_occupied");check(m.serialize()==old,"occupied mount consumed pouch");
            c.operation=Operation::SetState;c.item=10;c.mass=100;c.condition=4000;yes(m.execute(c));
            check(!m.storage_active(20)&&m.nodes().at(20).mounted_slot==1,"empty pouch detached on damage");
            c.operation=Operation::Erase;c.item=20;c.revision=m.revision();no(m.execute(c),"mounted_item");
        });
        test("forced rig wear retains inactive pouch and recovers full subtree with exact state",[]{
            auto a=root(1,Kind::ActorRoot,{1,1});auto r=item(10,1,100);r.container=true;r.grid={1,1};
            r.mounts={{101,"b1",{1,1},2,5000,true}};
            auto p=item(20,10,20);p.container=true;p.pouch=true;p.category=2;p.grid={2,2};p.location=Location::Mounted;p.mounted_slot=101;
            auto b=bag(30,20,30);b.attributes={{"serial","keep"}};auto x=item(31,30,7);Model m;yes(m.initialize({a,r,p,b,x}));
            Command damage;damage.operation=Operation::SetState;damage.item=10;damage.mass=100;damage.condition=4999;damage.session=m.session();
            auto old=m.serialize();no(m.execute(damage),"inactive_mount_contents");check(m.serialize()==old,"ordinary state partially spilled");
            yes(m.recover_rig_condition(10,4999,1,0,m.session()));
            check(m.revision()==1&&m.carried_mass()==157&&m.nodes().at(10).condition==4999,"damage or mass lost");
            check(m.nodes().at(20).mounted_slot==101&&!m.storage_active(20)&&m.cell_region(20,0,0)==0,"disabled grant exposed");
            check(m.nodes().at(30).location==Location::Overflow&&m.nodes().at(31).parent==30&&m.nodes().at(30).attributes==b.attributes,"recovery subtree state");
            auto saved=m.serialize();Model loaded;yes(loaded.deserialize(saved));check(loaded.serialize()==saved&&!loaded.storage_active(20),"inactive mount reload");
            no(move(m,{30,20,{0,0,false},1}),"inactive_mount_contents");
            yes(m.recover_rig_condition(10,5000,1,1,m.session()));check(m.storage_active(20)&&m.nodes().at(30).location==Location::Overflow,"repair moved contents implicitly");
            yes(move(m,{30,20,{0,0,false},2}));check(m.carried_mass()==157,"repair recovery mass");
        });
        test("rig condition recovery rejects foreign targets and stale requests atomically",[]{
            auto a=root(1,Kind::ActorRoot);auto w=root(2,Kind::WorldRoot);auto r=bag(10,2,100);r.mounts={{101,"b1",{1,1},2,5000,true}};
            Model m;yes(m.initialize({a,w,r}));auto old=m.serialize();
            no(m.recover_rig_condition(10,1,1,0,m.session()),"recovery_ownership");
            no(m.recover_rig_condition(10,1,1,0,0),"stale_session");
            no(m.recover_rig_condition(10,1,1,1,m.session()),"stale_revision");check(m.serialize()==old,"invalid recovery changed world");
            yes(move(m,{10,1,{0,0,false},0}));old=m.serialize();
            no(m.recover_rig_condition(10,10001,1,1,m.session()),"metadata");check(m.serialize()==old,"invalid condition changed state");
        });
        test("disabled mount rejects new attachment while retained inactive pouch survives",[]{
            auto a=root(1,Kind::ActorRoot);auto r=bag(10,1,100);r.soft=false;r.condition=4999;
            r.mounts={{101,"b1",{1,1},2,5000,true}};auto p=item(20,1,20,{5,0,false});p.container=true;p.pouch=true;p.category=2;p.grid={1,1};
            Model m;yes(m.initialize({a,r,p}));Command c;c.operation=Operation::Mount;c.item=20;c.destination=10;c.mount_slot=101;c.session=m.session();
            auto old=m.serialize();no(m.execute(c),"mount");
            c.operation=Operation::Spawn;c.spawned=p;c.spawned.id=21;c.spawned.parent=10;c.spawned.location=Location::Mounted;c.spawned.position={};c.spawned.mounted_slot=101;
            no(m.execute(c),"mount");check(m.serialize()==old,"inactive admission changed model");
        });
        test("loss of pouch grant recovers rig if new fullness collides without losing active contents",[]{
            auto a=root(1,Kind::ActorRoot,{2,1});auto r=bag(10,1,100);r.grid={2,1};r.half={1,1};r.full={2,2};
            r.mounts={{101,"b1",{1,1},2,5000,true}};
            auto p=item(20,10,20);p.container=true;p.pouch=true;p.category=2;p.grid={2,1};p.location=Location::Mounted;p.mounted_slot=101;
            auto x=item(30,10,7);x.empty={2,1};auto blocker=item(40,1,5,{1,0,false});Model m;yes(m.initialize({a,r,p,x,blocker}));
            check(m.footprint(10).h==1,"initial half size");yes(m.recover_rig_condition(10,4999,1,0,m.session()));
            check(m.nodes().at(10).location==Location::Overflow&&m.nodes().at(30).parent==10&&m.nodes().at(20).parent==10,"growth recovery lost rig contents");
            check(m.carried_mass()==132&&m.nodes().at(40).position.x==1,"recovery moved manual placement or mass");
        });
        test("empty rig dismantle returns pouches atomically and retains state",[]{
            auto a=root(1,Kind::ActorRoot);auto rig=item(10,1,100);rig.container=true;rig.grid={1,1};rig.mounts={{1,"b1",{1,1},2,0,true}};
            auto p=item(20,10,20);p.category=2;p.container=true;p.pouch=true;p.grid={1,2};p.location=Location::Mounted;p.mounted_slot=1;p.attributes={{"condition_note","worn-but-repaired"}};
            Model m;yes(m.initialize({a,rig,p}));Command moveP;moveP.item=20;moveP.destination=1;moveP.position={0,0,false};moveP.session=m.session();
            Command remove;remove.operation=Operation::Erase;remove.item=10;remove.session=m.session();
            // Ordered unmount needs a real free position; final-state swaps use relocate separately.
            moveP.position={2,0,false};yes(m.execute_batch({moveP,remove}));
            check(m.nodes().count(10)==0&&m.nodes().at(20).parent==1&&m.nodes().at(20).attributes.at("condition_note")=="worn-but-repaired","dismantle lost state");
        });
        test("per-instance state persists verbatim and incompatible members never merge",[]{
            auto a=root(1,Kind::ActorRoot);auto x=item(10,1,40);x.type=100;x.quantity=2;x.stack_limit=5;
            x.attributes={{"ammo","9x19:fmj,3"},{"parts","barrel=0.75;bolt=0.50"},{"upgrades","scope rail\noptic"}};
            auto y=x;y.id=11;y.position={2,0,false};y.attributes["ammo"]="9x19:ap,3";Model m;yes(m.initialize({a,x,y}));
            Command c;c.operation=Operation::Merge;c.item=10;c.destination=11;c.quantity=1;c.session=m.session();no(m.execute(c),"stack_compatibility");
            c.operation=Operation::SetState;c.item=10;c.mass=45;c.state=20;c.condition=7500;c.attributes=x.attributes;c.attributes["parts"]="barrel=0.95;bolt=0.50";yes(m.execute(c));
            auto saved=m.serialize();Model n;yes(n.deserialize(saved));check(n.serialize()==saved&&n.nodes().at(10).attributes==c.attributes&&n.nodes().at(10).condition==7500,"state restore");
            check(n.carried_mass()==170,"state mass update");
        });
        test("container-specific stack ceiling rejects full move but permits legal split",[]{
            auto a=root(1,Kind::ActorRoot);auto box=item(10,1,20);box.container=true;box.grid={2,2};box.max_stack_quantity=5;
            auto x=item(20,1,5,{3,0,false});x.type=100;x.quantity=10;x.stack_limit=30;Model m;yes(m.initialize({a,box,x}));
            rollback(m,{20,10,{0,0,false},0},"container_stack_limit");Command c;c.operation=Operation::Split;c.item=20;c.quantity=5;c.new_id=21;c.destination=10;c.session=m.session();yes(m.execute(c));
            check(m.nodes().at(20).quantity==5&&m.nodes().at(21).quantity==5&&m.carried_mass()==70,"domain split conservation");
        });
        test("explicit recovery retry preserves manual placements and publishes fitting items once",[]{
            auto a=root(1,Kind::ActorRoot,{3,1});auto fixed=item(10,1,10,{1,0,false});
            auto x=item(20,1,20);x.location=Location::Overflow;auto y=item(21,1,30);y.location=Location::Overflow;
            Model m;yes(m.initialize({a,fixed,x,y}));auto before=m.carried_mass();
            yes(m.retry_recovery(1,{1},0,m.session()));
            check(m.revision()==1&&m.nodes().at(10).position.x==1,"retry rearranged manual position");
            check(m.nodes().at(20).position.x==0&&m.nodes().at(21).position.x==2&&m.carried_mass()==before,"retry identity placement mass");
            auto saved=m.serialize();yes(m.retry_recovery(1,{1},1,m.session()));check(m.serialize()==saved,"no-op retry changed revision");
        });
        test("recovery retry uses requested owned storage and rotation without losing no-fit items",[]{
            auto a=root(1,Kind::ActorRoot,{1,1});auto b=item(10,1,10);b.container=true;b.grid={1,2};
            auto x=item(20,1,20);x.location=Location::Overflow;x.empty={2,1};auto y=x;y.id=21;y.empty={3,3};
            Model m;yes(m.initialize({a,b,x,y}));yes(m.retry_recovery(1,{10},0,m.session()));
            check(m.nodes().at(20).parent==10&&m.nodes().at(20).position.rotated,"retry rotated fit");
            check(m.nodes().at(21).location==Location::Overflow&&m.carried_mass()==50,"no-fit lost identity/mass");
            auto saved=m.serialize();Model loaded;yes(loaded.deserialize(saved));check(loaded.serialize()==saved,"retry persistence");
        });
        test("recovery retry rejects stale or foreign destination lists atomically",[]{
            auto a=root(1,Kind::ActorRoot);auto w=root(2,Kind::WorldRoot);auto x=item(10,1,7);x.location=Location::Overflow;
            Model m;yes(m.initialize({a,w,x}));auto old=m.serialize();
            no(m.retry_recovery(1,{1,2},0,m.session()),"recovery_ownership");
            no(m.retry_recovery(1,{1,1},0,m.session()),"recovery_destinations");
            no(m.retry_recovery(1,{},0,m.session()),"recovery_destinations");
            no(m.retry_recovery(1,{1},1,m.session()),"stale_revision");
            no(m.retry_recovery(1,{1},0,0),"stale_session");check(m.serialize()==old,"invalid retry published partial changes");
        });
        test("recovery retry cannot bypass category mask or self-ownership rules",[]{
            auto a=root(1,Kind::ActorRoot,{1,1});auto b=item(10,1,10);b.container=true;b.grid={2,1};b.cells={1,0};b.accepts=2;
            auto x=item(20,1,20);x.location=Location::Overflow;x.empty={2,1};x.category=2;
            auto y=item(21,1,30);y.location=Location::Overflow;y.container=true;y.grid={1,1};
            Model m;yes(m.initialize({a,b,x,y}));auto old=m.serialize();yes(m.retry_recovery(1,{10,21},0,m.session()));
            check(m.serialize()==old,"blocked grid/self destination bypassed");
        });
        test("native facts changing during drag reject stale release without moving items",[]{
            for(int change=0;change<3;++change) {
                Node x=item(100,1,100);x.attributes["native_section"]="pda";
                Node y=item(101,1,38);y.attributes["native_section"]="bandage";
                y.type=1;y.stack_limit=2;
                ActorLayout a;yes(a.reconcile({x,y}));DragSession drag;
                yes(drag.begin(a.model(),100));yes(a.reconcile({x,y}));check(drag.is_current(a.model()),"unchanged facts cancelled drag");
                std::vector<Node> facts{x,y};
                if(change==0)facts[1].condition=9000;
                if(change==1)facts[1].quantity=2;
                if(change==2){auto z=y;z.id=102;facts.push_back(z);}
                yes(a.reconcile(facts));auto before=a.model().serialize();
                auto proposal=drag.release(a.model(),1,5,4,false,{});
                no(proposal.result,"stale_drag");check(proposal.moves.empty()&&!drag.item(),"stale ghost/proposal survived");
                check(a.model().serialize()==before,"stale gesture changed refreshed inventory");
                yes(drag.begin(a.model(),100));proposal=drag.release(a.model(),1,5,4,false,{});
                yes(proposal.result);yes(a.move(proposal.moves.front()));
            }
        });
        test("native layout preserves manual position and stale guards across facts and reload",[]{
            Node n=item(100,1,100);n.attributes["native_section"]="pda";ActorLayout a;yes(a.reconcile({n}));
            auto rev=a.model().revision(),session=a.model().session();yes(a.move({100,1,{4,3,false},rev,session}));
            yes(a.reconcile({n}));check(a.model().nodes().at(100).position.x==4,"sync reset manual position");
            no(a.move({100,1,{5,3,false},rev,session}),"stale_revision");
            ActorLayout b;yes(b.restore(a.model().serialize()));n.intrinsic=150;n.condition=5000;yes(b.reconcile({n}));
            check(b.model().nodes().at(100).position.x==4&&b.model().carried_mass()==150&&b.model().nodes().at(100).condition==5000,"saved facts overrode native truth");
            no(b.move({100,1,{5,3,false},b.model().revision(),session}),"stale_session");
        });
        test("native layout reconciles arrivals removals and rejects identity mismatch atomically",[]{
            Node x=item(100,1,100);x.attributes["native_section"]="pda";Node y=x;y.id=101;ActorLayout a;yes(a.reconcile({x,y}));
            const auto pos=a.model().nodes().at(101).position;auto before=a.model().serialize();
            check(!a.move({100,1,pos,a.model().revision(),a.model().session()}),"overlap accepted");check(before==a.model().serialize(),"overlap changed state");
            x.attributes["native_section"]="wrong";no(a.reconcile({x,y}),"native_identity_section");check(a.model().serialize()==before,"mismatch published");
            yes(a.reconcile({y}));check(a.model().nodes().size()==3&&a.model().nodes().at(101).position.x==pos.x,"removal moved survivor");
            Model foreign;yes(foreign.initialize({root(3,Kind::ActorRoot)}));no(a.restore(foreign.serialize()),"native_layout_root");
            check(a.model().nodes().count(101)==1,"bad restore lost state");
        });
        test("native equipment and belt preserve carried mass outside bag placement",[]{
            Node p=item(100,1,100);p.attributes["native_section"]="pda";
            Node gun=item(101,1,3000);gun.attributes["native_section"]="gun";gun.location=Location::Equipment;gun.wear_slots=gun.equipped_slots=4;
            Node belt=item(102,1,500);belt.attributes={{"native_section","artifact"},{"native_place","2"}};
            ActorLayout layout;yes(layout.reconcile({p,gun,belt}));
            check(layout.model().nodes().at(101).location==Location::Equipment&&layout.model().nodes().at(102).location==Location::Equipment,"equipped item occupied bag");
            check(layout.model().carried_mass()==3600&&layout.model().nodes().at(1).grid.w==8,"native mass/grid");
            ActorLayout loaded;yes(loaded.restore(layout.model().serialize()));yes(loaded.reconcile({p,gun,belt}));
            gun.location=Location::Grid;gun.wear_slots=gun.equipped_slots=0;yes(loaded.reconcile({p,gun,belt}));
            check(loaded.model().nodes().at(101).location==Location::Grid&&loaded.model().carried_mass()==3600,"unequip recovery lost item");
        });
        test("logical native bags retain contents through unequip reload and native state changes",[]{
            auto bag=item(100,1,800);bag.container=true;bag.grid={4,4};bag.attributes["native_section"]="bag";
            bag.location=Location::Equipment;bag.wear_slots=bag.equipped_slots=8192;
            auto med=item(101,1,200);med.attributes["native_section"]="medkit";
            ActorStorage pockets{{4,2},{},true};ActorLayout a;yes(a.reconcile({bag,med},pockets));
            check(a.model().nodes().at(101).location==Location::Overflow,"forced arrival silently admitted");
            yes(a.move({101,100,{2,2,true},a.model().revision(),a.model().session()}));
            auto saved=a.model().serialize();ActorLayout b;yes(b.restore(saved));
            no(b.reconcile({bag,med}),"native_layout_mode");check(b.model().serialize()==saved,"flat adapter flattened saved forest");
            bag.location=Location::Grid;bag.equipped_slots=0;med.condition=4500;med.intrinsic=170;
            yes(b.reconcile({bag,med},pockets));
            check(b.model().nodes().at(101).parent==100&&b.model().nodes().at(101).position.y==2,"unequip flattened bag");
            check(b.model().nodes().at(100).location==Location::Overflow,"forced unequip hid recovery");
            check(b.model().carried_mass()==970&&b.model().subtree_mass(100)==970&&b.model().nodes().at(101).condition==4500,"nested mass/state stale or doubled");
            bag.location=Location::Equipment;bag.equipped_slots=8192;yes(b.reconcile({bag,med},pockets));
            check(b.model().nodes().at(101).parent==100&&b.model().nodes().at(100).location==Location::Equipment,"reequip reassigned contents");
            no(b.move({101,2,{0,0,false},b.model().revision(),b.model().session()}),"native_layout_owner");
        });
        test("logical native missing or resized storage keeps survivors in explicit recovery",[]{
            auto bag=item(100,1,800);bag.container=true;bag.grid={4,4};bag.attributes["native_section"]="bag";
            auto med=item(101,1,200);med.attributes["native_section"]="medkit";
            ActorStorage pockets{{4,2},{},true};ActorLayout a;yes(a.reconcile({bag,med},pockets));
            yes(a.move({101,100,{3,3,false},a.model().revision(),a.model().session()}));
            bag.grid={2,2};yes(a.reconcile({bag,med},pockets));
            check(a.model().nodes().at(101).location==Location::Overflow&&a.model().carried_mass()==1000,"shrink lost or silently repacked content");
            yes(a.move({101,100,{1,1,false},a.model().revision(),a.model().session()}));
            yes(a.reconcile({med},pockets));
            check(a.model().nodes().size()==3&&a.model().nodes().at(101).parent==1&&a.model().nodes().at(101).location==Location::Overflow&&a.model().carried_mass()==200,"removed bag lost surviving native child");
            ActorLayout b;yes(b.restore(a.model().serialize()));med.condition=7000;yes(b.reconcile({med},pockets));
            check(b.model().nodes().at(101).location==Location::Overflow,"reload silently admitted orphan");
        });
        test("logical native nested soft bags rebuild inside out and reject cycles atomically",[&]{
            auto bag=item(100,1,800);bag.container=true;bag.grid={4,4};bag.attributes["native_section"]="bag";
            auto pouch=item(101,1,100);pouch.container=true;pouch.grid={2,2};pouch.soft=true;pouch.half={2,1};pouch.full={2,2};pouch.attributes["native_section"]="pouch";
            auto med=item(102,1,200);med.attributes["native_section"]="medkit";
            ActorStorage pockets{{4,4},{},true};ActorLayout a;yes(a.reconcile({bag,pouch,med},pockets));
            yes(a.move({102,101,{0,0,false},a.model().revision(),a.model().session()}));
            yes(a.move({101,100,{1,1,false},a.model().revision(),a.model().session()}));
            yes(a.move({100,1,{2,2,false},a.model().revision(),a.model().session()}));
            auto saved=a.model().serialize();check(!a.move({100,101,{0,0,false},a.model().revision(),a.model().session()}),"cycle accepted");check(a.model().serialize()==saved,"cycle changed forest");
            yes(save_atomic(a.model(),std::string(argv[1])+".logical"));
            ActorLayout b;yes(b.restore(saved));med.intrinsic=225;yes(b.reconcile({med,bag,pouch},pockets));
            check(b.model().nodes().at(102).parent==101&&b.model().nodes().at(101).parent==100&&b.model().nodes().at(100).position.x==2,"nested reload flattened or moved bags");
            check(b.model().carried_mass()==1125&&b.model().subtree_mass(101)==325,"nested mass counted twice");
            // Native equipment is authoritative even if a weapon was logically stored.
            med.location=Location::Equipment;med.wear_slots=med.equipped_slots=4;yes(b.reconcile({bag,pouch,med},pockets));
            check(b.model().nodes().at(102).parent==1&&b.model().nodes().at(102).location==Location::Equipment,"native equip remained nested");
        });
        test("native storage view keeps owner coordinates separate from headers and recovery",[]{
            auto pack=item(100,1,800);pack.container=true;pack.grid={8,8};pack.attributes["native_section"]="backpack";
            auto med=item(101,1,200);med.attributes["native_section"]="medkit";
            ActorLayout a;yes(a.reconcile({pack,med},{{4,2},{},true}));
            yes(a.move({101,100,{2,3,true},a.model().revision(),a.model().session()}));
            auto before=a.model().serialize();NativeStorageView view;yes(view.build(a.model()));
            Position p;check(view.position(a.model().nodes().at(101),p)&&p.x==2&&p.y==7&&p.rotated,"nested coordinate projection");
            Id owner=0;Position local;check(view.destination(p.x,p.y,owner,local)&&owner==100&&local.y==3,"drop did not recover logical owner");
            check(!view.destination(4,1,owner,local)&&!view.destination(0,0,owner,local)&&!view.destination(0,3,owner,local),"header or nonexistent pocket accepted");
            check(view.position(a.model().nodes().at(100),p)&&p.y==13&&!view.destination(p.x,p.y,owner,local),"recovery became legal capacity");
            check(before==a.model().serialize(),"view changed ownership or mass");
        });
        test("authored native capacity shrink retains items and mass in recovery",[]{
            Node rifle=item(100,1,3000);rifle.empty=rifle.half=rifle.full={5,2};rifle.attributes["native_section"]="rifle";
            Node med=item(101,1,500);med.attributes["native_section"]="medkit";
            ActorLayout layout;yes(layout.reconcile({rifle,med},{{8,8},{}}));
            DragSession drag;yes(drag.begin(layout.model(),101));
            yes(layout.reconcile({rifle,med},{{4,2},{}}));
            check(!drag.is_current(layout.model()),"capacity change kept old gesture");
            check(layout.model().nodes().at(100).location==Location::Overflow,"long rifle fit bare pockets");
            check(layout.model().nodes().at(101).location==Location::Grid&&layout.model().carried_mass()==3500,"shrink lost item or mass");
            ActorLayout loaded;yes(loaded.restore(layout.model().serialize()));yes(loaded.reconcile({rifle,med},{{4,2},{}}));
            check(loaded.model().nodes().at(100).location==Location::Overflow,"reload lost recovery");
            yes(loaded.reconcile({rifle,med},{{8,8},{}}));
            check(loaded.model().nodes().at(100).location==Location::Grid&&loaded.model().carried_mass()==3500,"expanded recovery failed");
        });
        test("capacity preview normalizes projected positions before recovery staging",[]{
            auto rifle=item(100,1,3000);rifle.empty=rifle.half=rifle.full={5,2};rifle.attributes["native_section"]="rifle";
            auto med=item(101,1,500);med.attributes["native_section"]="medkit";
            ActorLayout active;yes(active.reconcile({rifle,med},{{8,8},{}}));
            yes(active.move({100,1,{5,2,true},active.model().revision(),active.model().session()}));
            auto original=active.model().serialize();
            std::vector<Node> projected;
            for(const auto& pair:active.model().nodes())if(pair.second.kind==Kind::Item)projected.push_back(pair.second);
            ActorLayout preview;yes(preview.restore(original));yes(preview.reconcile(projected,{{4,2},{}}));
            check(preview.model().nodes().at(100).location==Location::Overflow,"projected rifle escaped recovery");
            check(preview.model().nodes().at(100).position.x==0&&!preview.model().nodes().at(100).position.rotated,"recovery retained grid coordinates");
            check(preview.model().carried_mass()==3500&&active.model().serialize()==original,"preview mutated mass or active layout");
            yes(preview.reconcile(projected,{{8,8},{}}));
            check(preview.model().nodes().at(100).location==Location::Grid&&preview.model().carried_mass()==3500,"projected recovery could not expand");
        });
        test("native admission preview respects masks rotation and full storage without mutation",[]{
            ActorLayout layout;ActorStorage pockets{{4,2},{1,1,1,1,1,1,1,0}};
            std::vector<Node> live;
            for(Id id=100;id<107;++id){auto n=item(id,1,10);n.attributes["native_section"]="small";live.push_back(n);}
            yes(layout.reconcile(live,pockets));auto before=layout.model().serialize();
            no(layout.preview_ruck(item(999,1,20)).result,"capacity");
            check(layout.model().serialize()==before,"refused admission mutated inventory");
            live.pop_back();yes(layout.reconcile(live,pockets));before=layout.model().serialize();
            auto fit=layout.preview_ruck(item(999,1,20));yes(fit.result);
            check(fit.position.x==2&&fit.position.y==1&&!fit.position.rotated,"partial-row admission used blocked cell");
            check(layout.model().serialize()==before,"accepted preview mutated inventory");
            ActorLayout narrow;yes(narrow.reconcile({},{{2,3},{}}));auto tall=item(900,1,25);tall.empty=tall.half=tall.full={3,2};
            fit=narrow.preview_ruck(tall);yes(fit.result);check(fit.position.rotated,"rotation-only admission refused");
            auto invalid=narrow.model().serialize();check(!narrow.reconcile({},{{0,3},{}}),"zero capacity accepted");
            check(narrow.model().serialize()==invalid,"invalid policy destroyed layout");
        });
        test("authored backpack sizes preserve rows-first geometry and partial cells",[]{
            ActorStorage shape;yes(parse_storage_shape("15x8",7,shape));
            check(shape.grid.w==8&&shape.grid.h==15&&shape.cells.empty(),"tourist shape transposed");
            yes(parse_storage_shape(" 2 X 11 ",7,shape));check(shape.grid.w==11&&shape.grid.h==2,"wide backpack clamped");
            yes(parse_storage_shape("9",4,shape));
            check(shape.grid.w==4&&shape.grid.h==3&&shape.cells.size()==12&&shape.cells[8]==1&&shape.cells[9]==0,"partial count grants cells");
            for(const char* invalid:{"0","-1","2x0","65x2","2x65","1x2x3","900000000000000000000","3.5","12garbage","1 2"}) {
                const auto before=shape.cells;no(parse_storage_shape(invalid,4,shape),"storage_shape");
                check(shape.grid.w==4&&shape.grid.h==3&&shape.cells==before,"invalid shape changed output");
            }
        });
        test("storage catalog selects equipped section then kind then authored default",[]{
            StorageCatalog catalog;ActorStorage sack,tourist,family;
            yes(parse_storage_shape("8x8",7,sack));yes(parse_storage_shape("15x8",7,tourist));yes(parse_storage_shape("4x11",7,family));
            catalog.sections["itm_backpack"]=sack;catalog.sections["equ_tourist_pack"]=tourist;catalog.kinds["i_backpack"]=family;
            check(catalog.resolve("","i_backpack").grid.w==4&&catalog.resolve("","i_backpack").grid.h==2,"spare pack granted worn storage");
            check(catalog.resolve("itm_backpack","i_backpack").grid.h==8,"kind overrode exact section");
            check(catalog.resolve("equ_tourist_pack","").grid.h==15,"tourist catalog capacity");
            check(catalog.resolve("addon_pack","i_backpack").grid.w==11,"kind fallback lost width");
            check(catalog.resolve("addon_pack","unknown").grid.w==7&&catalog.resolve("addon_pack","unknown").grid.h==10,"default fallback");
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
