#include "../../../src/inventory_next/native_layout.h"
#include "../../../src/inventory_next/native_storage_view.h"
#include "../../../src/inventory_next/storage_catalog.h"
#include "model.h"
#include "access_gate.h"
#include "identity_ledger.h"
#include "identity_state.h"
#include "transfer_session.h"
#include "world_subtrees.h"
#include "loaded_storage_session.h"
#include "ground_pickup.h"
#include "purchase_session.h"
#include "equipment_registry.h"
#include "quick_access.h"
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
        if(argc==3 && std::string(argv[1])=="--read-world") {
            std::ifstream in(argv[2],std::ios::binary);check(bool(in),"world fixture missing");
            const std::string bytes((std::istreambuf_iterator<char>(in)),{});
            WorldSubtrees restored;yes(restored.restore(bytes));check(restored.serialize()==bytes,"world ledger restart changed encoding");
            const auto& record=restored.records().at(100);const auto& nodes=record.tree.nodes();
            check(record.custodian==900&&nodes.at(101).parent==100&&nodes.at(102).parent==101,"world restart lost custody or nested owners");
            check(record.tree.carried_mass()==0&&record.tree.subtree_mass(100)==150&&nodes.at(102).condition==8314&&nodes.at(102).attributes.at("ammo")=="5","world restart lost state or mass");
            std::cout<<"PASS independent detached world subtree restart\n";return 0;
        }
        if(argc==3 && std::string(argv[1])=="--read-custom-equipment") {
            Model saved;yes(load_file(saved,argv[2]));ActorLayout restored;yes(restored.restore(saved.serialize()));
            const auto& nodes=restored.model().nodes();
            check(nodes.at(100).location==Location::Equipment&&nodes.at(100).equipped_slots==(std::uint64_t{1}<<19),"custom root equipment lost");
            check(nodes.at(101).location==Location::Mounted&&nodes.at(101).parent==100&&nodes.at(101).mounted_slot==65537,"custom pouch binding lost");
            check(nodes.at(102).parent==101&&nodes.at(100).attributes.at("native_custom_slot")=="65536"&&restored.model().carried_mass()==1600,"custom equipment restart changed state");
            std::cout<<"PASS independent custom equipment and mounted pouch restart\n";return 0;
        }
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
        test("logical backpack pickup acknowledges physical actor while preserving logical destination",[]{
            auto pack=item(10,1,50);pack.container=true;pack.grid={3,3};
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),pack,item(11,2,7)}));
            EntityRegistry registry;for(Id id:{1,2,10,11}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id+100)));}
            AccessGate access(m);yes(access.observe(2,true,true));yes(access.observe_item(11,true,true));
            Command c;c.operation=Operation::Move;c.item=11;c.destination=10;c.session=m.session();
            TransferSession transfer(m,access,registry);TransferSession::PhysicalOwners physical;physical.destination=registry.handle(1);
            auto before=m.serialize();yes(transfer.prepare({c,access.revision()},false,physical));
            TransferSession::Dispatch event;yes(transfer.dispatch(event));
            check(event.destination.live==101&&event.command.destination==10&&m.serialize()==before,"physical and logical destination conflated");
            yes(transfer.acknowledge(event.token,101,true));
            check(m.nodes().at(11).parent==10&&m.carried_mass()==57,"actor acknowledgment flattened destination or doubled weight");
            // Returning a nested object still physically detaches from the actor.
            c.destination=2;c.revision=m.revision();physical={};physical.source=registry.handle(1);
            yes(transfer.prepare({c,access.revision()},false,physical));yes(transfer.dispatch(event));
            check(event.source&&event.source->live==101,"nested source used bag as physical owner");
            no(transfer.acknowledge(event.token,101,false),"native_rejected");
            check(m.nodes().at(11).parent==10&&m.carried_mass()==57&&transfer.can_save(),"rejected native release altered subtree");
        });
        test("physical ownership override rejects foreign actors and stale intermediate containers",[]{
            auto pack=item(10,1,50);pack.container=true;pack.grid={3,3};
            auto pouch=item(11,10,5);pouch.container=true;pouch.grid={2,2};
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),root(3,Kind::ActorRoot),pack,pouch,item(12,2,7)}));
            EntityRegistry registry;for(Id id:{1,2,3,10,11,12}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
            AccessGate access(m);yes(access.observe(2,true,true));yes(access.observe_item(12,true,true));
            Command c;c.operation=Operation::Move;c.item=12;c.destination=11;c.session=m.session();
            TransferSession transfer(m,access,registry);TransferSession::PhysicalOwners physical;physical.destination=registry.handle(3);
            auto before=m.serialize();no(transfer.prepare({c,access.revision()},false,physical),"native_physical_owner");
            physical.destination=registry.handle(1);yes(transfer.prepare({c,access.revision()},false,physical));
            yes(registry.unbind(*registry.handle(10)));TransferSession::Dispatch event;no(transfer.dispatch(event),"stale_entity");
            check(m.serialize()==before&&transfer.can_save(),"stale ancestor mutated or locked source");
            yes(registry.bind(10,10));yes(transfer.prepare({c,access.revision()},false,physical));yes(transfer.dispatch(event));
            no(transfer.acknowledge(event.token,11,true),"native_parent_mismatch");
            check(m.serialize()==before&&!transfer.can_save(),"logical bag ID accepted as physical native parent");
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
        test("loaded native custody moves every descendant before publishing and survives roundtrip",[]{
            auto shell=bag(10,1,100);auto inner=bag(11,10,20);auto child=item(12,11,30);
            child.condition=8314;child.attributes={{"ammo","5"},{"serial","kept"}};
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),shell,inner,child}));
            EntityRegistry registry;for(Id id:{1,2,10,11,12}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id+100)));}
            AccessGate access(m);yes(access.observe(2,true,true));yes(access.observe_item(10,true,true));
            TransferSession transfer(m,access,registry);TransferSession::PhysicalOwners physical;
            physical.flatten_source=true;physical.flatten_destination=true;
            for(Id destination:{2,1}) {
                Command c;c.item=10;c.destination=destination;c.session=m.session();c.revision=m.revision();
                const auto before=m.serialize();const auto revision=m.revision();
                yes(transfer.prepare({c,access.revision()},false,physical));TransferSession::Dispatch event;yes(transfer.dispatch(event));
                check(event.subtree.size()==3&&!transfer.can_save()&&m.serialize()==before,"subtree published before all acknowledgments");
                no(transfer.acknowledge(event.token,event.destination.live,true),"subtree_ack_required");
                std::vector<TransferSession::ParentObservation> observed;
                for(const auto& member:event.subtree) {
                    check(member.source&&member.destination&&member.destination->live==destination+100,"wrong flattened custody plan");
                    observed.insert(observed.begin(),{member.item,member.destination->live});
                }
                yes(transfer.acknowledge_subtree(event.token,observed,true));
                check(m.revision()==revision+1&&transfer.can_save(),"subtree did not publish exactly once");
                check(m.nodes().at(10).parent==destination&&m.nodes().at(11).parent==10&&m.nodes().at(12).parent==11,"logical subtree flattened");
                check(m.carried_mass()==(destination==1?150:0)&&m.subtree_mass(10)==150,"subtree mass lost or counted twice");
                check(m.nodes().at(12).condition==8314&&m.nodes().at(12).attributes==child.attributes,"contained state lost");
                Model saved;yes(saved.deserialize(m.serialize()));check(saved.serialize()==m.serialize(),"world subtree persistence changed state");
                no(transfer.acknowledge_subtree(event.token,observed,true),"transfer_ack");
            }
        });
        test("partial duplicate and wrong-parent subtree acknowledgments block saves without publishing",[]{
            for(int failure=0;failure<3;++failure) {
                Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(10,1,100),item(11,10,20)}));
                EntityRegistry registry;for(Id id:{1,2,10,11}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
                AccessGate access(m);yes(access.observe(2,true,true));TransferSession transfer(m,access,registry);
                TransferSession::PhysicalOwners physical;physical.flatten_source=true;physical.flatten_destination=true;
                Command c;c.item=10;c.destination=2;c.session=m.session();const auto before=m.serialize();
                yes(transfer.prepare({c,access.revision()},false,physical));TransferSession::Dispatch event;yes(transfer.dispatch(event));
                std::vector<TransferSession::ParentObservation> observed{{*registry.handle(10),2},{*registry.handle(11),2}};
                if(failure==0)observed.pop_back();
                if(failure==1)observed[1]=observed[0];
                if(failure==2)observed[1].parent=1;
                no(transfer.acknowledge_subtree(event.token,observed,true),failure==0?"native_subtree_incomplete":failure==1?"native_subtree_identity":"native_parent_mismatch");
                check(m.serialize()==before&&!transfer.can_save()&&transfer.state()==TransferSession::State::ReconcileRequired,"partial native move published or became saveable");
                no(transfer.cancel(),"native_reconcile_required");no(transfer.reconciled(),"rebuild_required");
                yes(m.deserialize(before));yes(transfer.reconciled());check(transfer.can_save(),"rebuilt ownership remained locked");
            }
        });
        test("subtree rejection requires every descendant restored to original native custody",[]{
            for(bool restored:{false,true}) {
                Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(10,1,100),item(11,10,20)}));
                EntityRegistry registry;for(Id id:{1,2,10,11}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
                AccessGate access(m);yes(access.observe(2,true,true));TransferSession transfer(m,access,registry);
                TransferSession::PhysicalOwners physical;physical.flatten_source=true;physical.flatten_destination=true;
                Command c;c.item=10;c.destination=2;c.session=m.session();const auto before=m.serialize();
                yes(transfer.prepare({c,access.revision()},false,physical));TransferSession::Dispatch event;yes(transfer.dispatch(event));
                no(transfer.acknowledge_subtree(event.token,{{*registry.handle(10),1},{*registry.handle(11),static_cast<std::uint16_t>(restored?1:2)}},false),restored?"native_rejected":"native_parent_mismatch");
                check(m.serialize()==before&&transfer.can_save()==restored,"rejected subtree lost source or unlocked partial native state");
            }
        });
        test("descendant live ID reuse invalidates complete subtree acknowledgment",[]{
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(10,1,100),item(11,10,20)}));
            EntityRegistry registry;for(Id id:{1,2,10,11}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
            AccessGate access(m);yes(access.observe(2,true,true));TransferSession transfer(m,access,registry);
            Command c;c.item=10;c.destination=2;c.session=m.session();const auto before=m.serialize();
            yes(transfer.prepare({c,access.revision()}));TransferSession::Dispatch event;yes(transfer.dispatch(event));
            const auto old=*registry.handle(11);yes(registry.unbind(old));yes(registry.bind(11,11));
            no(transfer.acknowledge_subtree(event.token,{{*registry.handle(10),2},{*registry.handle(11),10}},true),"stale_entity");
            check(m.serialize()==before&&!transfer.can_save(),"reused descendant slot authorized transfer");
        });
        test("natural subtree custody preserves child native parent and flat item endpoints are refused",[]{
            auto target=bag(20,2,10);target.soft=false;target.grid={4,4};
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(10,1,100),item(11,10,20),target}));
            EntityRegistry registry;for(Id id:{1,2,10,11,20}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
            AccessGate access(m);yes(access.observe(2,true,true));yes(access.observe_item(20,true,true));TransferSession transfer(m,access,registry);
            Command c;c.item=10;c.destination=20;c.session=m.session();TransferSession::PhysicalOwners physical;physical.flatten_destination=true;
            no(transfer.prepare({c,access.revision()},false,physical),"native_subtree_owner");
            physical.destination=registry.handle(2);yes(transfer.prepare({c,access.revision()},false,physical));yes(transfer.cancel());
            yes(transfer.prepare({c,access.revision()}));TransferSession::Dispatch event;yes(transfer.dispatch(event));
            check(event.subtree.size()==2&&event.subtree[1].destination->live==10,"natural child custody changed");
            yes(transfer.acknowledge_subtree(event.token,{{*registry.handle(10),20},{*registry.handle(11),10}},true));
            check(m.nodes().at(10).parent==20&&m.nodes().at(11).parent==10&&m.carried_mass()==0,"natural subtree transfer lost nested structure");
        });
        test("detached world subtree persistence retains nested identity layout and state",[&]{
            auto shell=bag(100,2,100);auto inner=bag(101,100,20);auto child=item(102,101,30,{1,0,true});
            child.condition=8314;child.attributes={{"ammo","5"},{"section","wpn_pm"}};
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),shell,inner,child,item(103,1,7)}));
            WorldSubtrees ledger;yes(ledger.capture(m,100,900));const auto before=m.serialize();
            const auto& saved=ledger.records().at(100).tree;
            check(saved.nodes().size()==5&&saved.carried_mass()==0&&saved.subtree_mass(100)==150,"world archive included actor items or lost weight");
            check(saved.nodes().at(102).parent==101&&saved.nodes().at(102).position.rotated&&saved.nodes().at(102).position.x==1&&saved.nodes().at(102).attributes==child.attributes,"world archive changed interior layout/state");
            WorldSubtrees loaded;yes(loaded.restore(ledger.serialize()));check(loaded.serialize()==ledger.serialize()&&m.serialize()==before,"archive load mutated source or record");
            std::ofstream out(std::string(argv[1])+".world",std::ios::binary);out<<ledger.serialize();check(bool(out),"world fixture write failed");
        });
        test("world ledger refuses actor capture duplicate descendants and wrong custody release",[]{
            Model carried;yes(carried.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(100,1,100),item(101,100,20)}));
            WorldSubtrees ledger;no(ledger.capture(carried,100,900),"world_storage_owner");
            yes(move(carried,{100,2,{0,0,false},carried.revision()}));yes(ledger.capture(carried,100,900));const auto before=ledger.serialize();
            no(ledger.capture(carried,100,900),"world_storage_duplicate");no(ledger.release(100,901),"world_storage_custodian");
            Model overlapping;yes(overlapping.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(110,2,100),item(101,110,20)}));
            no(ledger.capture(overlapping,110,901),"world_storage_identity");check(ledger.serialize()==before,"invalid archive operation changed ledger");
            yes(ledger.release(100,900));check(ledger.records().empty(),"acknowledged release retained duplicate archive");
        });
        test("world ledger corrupt truncated and forged custody data cannot replace valid records",[]{
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(100,2,100),item(101,100,20)}));
            WorldSubtrees ledger;yes(ledger.capture(m,100,900));const auto before=ledger.serialize();
            auto corrupt=before;corrupt[corrupt.find("900")]='8';no(ledger.restore(corrupt),"world_storage_checksum");
            check(!ledger.restore(before.substr(0,before.size()-1)),"truncated world ledger accepted");
            no(ledger.capture(m,100,101),"world_storage_custodian");
            check(ledger.serialize()==before,"failed world restore replaced valid records");
        });
        test("world ledger supports distinct ground and crate bags without merging their items",[]{
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(100,2,100),item(101,100,20),bag(110,2,200,{3,0,false}),item(111,110,30)}));
            WorldSubtrees ledger;yes(ledger.capture(m,100,0));yes(ledger.capture(m,110,900));
            WorldSubtrees loaded;yes(loaded.restore(ledger.serialize()));check(loaded.records().size()==2&&loaded.records().at(100).custodian==0&&loaded.records().at(110).custodian==900,"world custodians merged");
            check(loaded.records().at(100).tree.subtree_mass(100)==120&&loaded.records().at(110).tree.subtree_mass(110)==230,"world records mass merged");
            yes(loaded.release(100,0));check(loaded.records().size()==1&&loaded.records().count(110),"ground release removed other bag");
        });
        test("world storage refresh uses actual native state while retaining nested placement",[]{
            auto shell=bag(100,2,100);shell.attributes["native_section"]="itm_backpack";
            auto child=item(101,100,20,{1,0,true});child.attributes={{"native_section","wpn_pm"},{"ammo","5"}};
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),shell,child}));
            WorldSubtrees ledger;yes(ledger.capture(m,100,900));
            auto actual=child;actual.parent=2;actual.position={};actual.condition=7500;actual.intrinsic=25;actual.attributes["ammo"]="3";
            yes(ledger.refresh(100,900,{shell,actual}));const auto& node=ledger.records().at(100).tree.nodes().at(101);
            check(node.parent==100&&node.position.x==1&&node.position.rotated&&node.condition==7500&&node.intrinsic==25&&node.attributes.at("ammo")=="3","saved state overrode native facts or placement lost");
            check(ledger.records().at(100).tree.subtree_mass(100)==125,"world native mass refresh failed");
        });
        test("named stash survives native refresh and restart without changing item facts",[]{
            auto shell=bag(100,2,100);shell.attributes["native_section"]="itm_backpack";
            auto child=item(101,100,20,{1,0,true});child.attributes["native_section"]="medkit";
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),shell,child}));
            WorldSubtrees world;yes(world.capture(m,100,900));yes(world.name_stash(100,900,"Cordon supplies"));
            child.condition=8000;yes(world.refresh(100,900,{shell,child}));
            WorldSubtrees restored;yes(restored.restore(world.serialize()));
            check(restored.stash_name(100)=="Cordon supplies"&&restored.records().at(100).custodian==900,"stash label or persistent custody lost");
            const auto& saved=restored.records().at(100).tree.nodes().at(101);
            check(saved.condition==8000&&saved.parent==100&&saved.position.rotated&&saved.attributes==child.attributes,"stash metadata changed native contents");
        });
        test("stash naming rejects stale custody and malformed text atomically",[]{
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(100,2,100)}));
            WorldSubtrees world;yes(world.capture(m,100,900));yes(world.name_stash(100,900,"Original"));const auto before=world.serialize();
            no(world.name_stash(100,901,"Wrong owner"),"world_storage_custodian");
            for(const auto& name:std::vector<std::string>{"",std::string(241,'x'),"line\nbreak",std::string("nul\0byte",8)})
                no(world.name_stash(100,900,name),"world_storage_stash_name");
            check(world.serialize()==before&&world.stash_name(999).empty(),"invalid name changed world custody");
        });
        test("explicit empty stash roundtrip keeps original shell and discards world-only label",[]{
            auto shell=bag(100,1,100);shell.condition=8314;shell.attributes={{"native_section","itm_backpack"},{"upgrade","retained"}};
            Model actor;yes(actor.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),shell}));
            WorldSubtrees world;EntityRegistry registry;
            for(Id id:{1,100,900}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
            LoadedStorageSession release(actor,world,registry);yes(release.prepare_release(100,900,"Empty stash"));TransferSession::Dispatch event;yes(release.dispatch(true,event));
            yes(release.acknowledge(event.token,{{*registry.handle(100),900}},true));actor=*release.committed_actor();world=*release.committed_world();
            check(world.stash_name(100)=="Empty stash","acknowledged stash name missing");
            LoadedStorageSession take(actor,world,registry);yes(take.prepare_acquire(100,900,{1}));yes(take.dispatch(true,event));yes(take.acknowledge(event.token,{{*registry.handle(100),1}},true));
            actor=*take.committed_actor();world=*take.committed_world();
            check(world.records().empty()&&actor.nodes().size()==3&&actor.carried_mass()==100,"empty stash duplicated shell or lost mass");
            check(actor.nodes().at(100).condition==shell.condition&&actor.nodes().at(100).attributes==shell.attributes&&actor.nodes().at(2).attributes.empty(),"empty stash recreated item state or leaked naming metadata");
        });
        test("invalid stash naming refuses before native dispatch or publication",[]{
            Model actor;yes(actor.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(100,1,100)}));
            WorldSubtrees world;EntityRegistry registry;for(Id id:{1,100,900}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
            const auto before=actor.serialize();LoadedStorageSession release(actor,world,registry);
            no(release.prepare_release(100,900,std::string(241,'x')),"world_storage_stash_name");
            TransferSession::Dispatch event;no(release.dispatch(true,event),"transfer_state");
            check(release.can_save()&&!release.committed_actor()&&world.records().empty()&&actor.serialize()==before,"invalid named stash changed source or blocked saving");
        });
        test("world storage refresh rejects missing replaced and resized contents atomically",[]{
            auto shell=bag(100,2,100);shell.attributes["native_section"]="itm_backpack";
            auto child=item(101,100,20,{1,0,true});child.attributes["native_section"]="wpn_pm";
            Model m;yes(m.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),shell,child}));
            WorldSubtrees ledger;yes(ledger.capture(m,100,900));const auto before=ledger.serialize();
            no(ledger.refresh(100,901,{shell,child}),"world_storage_custodian");
            no(ledger.refresh(100,900,{shell}),"world_storage_incomplete");
            no(ledger.refresh(100,900,{shell,shell}),"world_storage_identity");
            auto changed=child;changed.attributes["native_section"]="wpn_ak74";
            no(ledger.refresh(100,900,{shell,changed}),"world_storage_section");
            changed=child;changed.empty={3,3};check(!ledger.refresh(100,900,{shell,changed}),"resized subtree accepted invalid placement");
            check(ledger.serialize()==before,"failed native refresh altered ledger");
        });
        test("loaded actor to crate and back commits actor and world candidates together",[]{
            auto shell=bag(100,1,100);auto inner=bag(101,100,20);auto child=item(102,101,30);
            child.attributes={{"ammo","9"}};child.condition=9098;
            Model actor;yes(actor.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),shell,inner,child}));
            WorldSubtrees world;EntityRegistry registry;
            for(Id id:{1,100,101,102,900}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
            LoadedStorageSession release(actor,world,registry);yes(release.prepare_release(100,900));
            TransferSession::Dispatch event;yes(release.dispatch(true,event));
            check(event.subtree.size()==3&&!release.committed_actor()&&!release.committed_world()&&!release.can_save(),"loaded transfer published early");
            yes(release.acknowledge(event.token,{{*registry.handle(100),900},{*registry.handle(101),900},{*registry.handle(102),900}},true));
            actor=*release.committed_actor();world=*release.committed_world();
            check(actor.carried_mass()==0&&actor.nodes().size()==2&&world.records().at(100).tree.subtree_mass(100)==150,"loaded release lost or doubled ownership");
            LoadedStorageSession take(actor,world,registry);yes(take.prepare_acquire(100,900,{1}));yes(take.dispatch(true,event));
            yes(take.acknowledge(event.token,{{*registry.handle(102),1},{*registry.handle(100),1},{*registry.handle(101),1}},true));
            actor=*take.committed_actor();world=*take.committed_world();
            check(world.records().empty()&&actor.carried_mass()==150&&actor.nodes().at(101).parent==100&&actor.nodes().at(102).parent==101,"loaded pickup flattened tree or retained duplicate archive");
            check(actor.nodes().at(102).condition==9098&&actor.nodes().at(102).attributes==child.attributes,"loaded roundtrip changed contained weapon state");
        });
        test("loaded pickup preflights complete bag footprint and preserves full actor and source",[]{
            Model source;yes(source.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(100,2,100),item(101,100,20)}));
            WorldSubtrees world;yes(world.capture(source,100,900));
            Model actor;yes(actor.initialize({root(1,Kind::ActorRoot,{1,1}),root(2,Kind::WorldRoot),item(110,1,7)}));
            EntityRegistry registry;for(Id id:{1,100,101,110,900}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
            const auto before=actor.serialize(),outside=world.serialize();LoadedStorageSession take(actor,world,registry);
            no(take.prepare_acquire(100,900,{1}),"no_fit");
            check(take.can_save()&&!take.committed_actor()&&actor.serialize()==before&&world.serialize()==outside,"full loaded pickup altered source");
        });
        test("loaded transfer rejects partial native changes and stale actor or world ledgers",[]{
            for(int failure=0;failure<3;++failure) {
                Model actor;yes(actor.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(100,1,100),item(101,100,20)}));
                WorldSubtrees world;EntityRegistry registry;for(Id id:{1,100,101,900}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
                LoadedStorageSession release(actor,world,registry);yes(release.prepare_release(100,900));TransferSession::Dispatch event;yes(release.dispatch(true,event));
                if(failure==1){Command changed;changed.operation=Operation::SetMass;changed.item=101;changed.mass=21;changed.session=actor.session();changed.revision=actor.revision();yes(actor.execute(changed));}
                if(failure==2){Model extra;yes(extra.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(200,2,30)}));yes(world.capture(extra,200,901));}
                const auto before=actor.serialize(),outside=world.serialize();
                no(release.acknowledge(event.token,{{*registry.handle(100),900},{*registry.handle(101),static_cast<std::uint16_t>(failure==0?1:900)}},true),failure==0?"native_parent_mismatch":failure==1?"stale_model":"stale_world_storage");
                check(!release.can_save()&&!release.committed_actor()&&!release.committed_world()&&actor.serialize()==before&&world.serialize()==outside,"uncertain loaded transfer published candidates");
                no(release.dispatch(false,event),"transfer_state");check(!release.can_save(),"repeated dispatch erased pending fault");
            }
        });
        test("loaded transfer loss of access before dispatch cancels without moving records",[]{
            Model actor;yes(actor.initialize({root(1,Kind::ActorRoot),root(2,Kind::WorldRoot),bag(100,1,100),item(101,100,20)}));
            WorldSubtrees world;EntityRegistry registry;for(Id id:{1,100,101,900}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));}
            LoadedStorageSession release(actor,world,registry);yes(release.prepare_release(100,900));TransferSession::Dispatch event;
            const auto before=actor.serialize();no(release.dispatch(false,event),"world_unavailable");
            check(release.can_save()&&actor.serialize()==before&&world.records().empty(),"lost access changed loaded ownership");
            no(release.prepare_release(100,0),"world_storage_custodian");
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
            ActorLayout b;yes(b.restore(saved));med.intrinsic=225;
            bag.attributes["storage_allow_container_items"]="0";
            yes(b.reconcile({med,bag,pouch},pockets));
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
            Id owner=0;Position local;check(!view.destination(p.x,p.y,owner,local),"recovery ancestor accepted new contents");
            check(!view.destination(4,1,owner,local)&&!view.destination(0,0,owner,local)&&!view.destination(0,3,owner,local),"header or nonexistent pocket accepted");
            check(view.position(a.model().nodes().at(100),p)&&p.y==13&&!view.destination(p.x,p.y,owner,local),"recovery became legal capacity");
            check(before==a.model().serialize(),"view changed ownership or mass");
            yes(a.move({100,1,{0,0,false},a.model().revision(),a.model().session()}));yes(view.build(a.model()));
            check(view.position(a.model().nodes().at(101),p)&&view.destination(p.x,p.y,owner,local)&&owner==100&&local.y==3,"recovered bag did not restore local destination");
        });
        test("recovery container windows permit retrieval display but never extra manual capacity",[]{
            auto pack=item(100,1,800);pack.container=true;pack.grid={8,8};pack.empty=pack.half=pack.full={5,5};
            auto med=item(101,1,200);ActorLayout a;yes(a.reconcile({pack,med},{{4,2},{},true}));
            check(a.model().nodes().at(100).location==Location::Overflow,"fixture needs recovery pack");
            yes(a.move({101,100,{1,1,false},a.model().revision(),a.model().session()}));
            StorageViewOptions options;options.owner=100;NativeStorageView view;yes(view.build(a.model(),options));
            Position shown;check(view.position(a.model().nodes().at(101),shown),"recovery contents inaccessible for retrieval");
            Id owner=0;Position local;check(!view.destination(3,3,owner,local),"recovery pack became extra capacity");
        });
        test("cross-window drop preview and commit share local coordinates without preview mutation",[]{
            auto outer=item(100,1,800);outer.container=true;outer.grid={8,8};outer.empty=outer.half=outer.full={2,2};
            auto inner=item(101,1,300);inner.container=true;inner.grid={4,4};auto med=item(102,1,200);
            ActorLayout a;yes(a.reconcile({outer,inner,med},{{4,2},{},true}));
            yes(a.move({101,100,{1,1,false},a.model().revision(),a.model().session()}));
            yes(a.move({102,101,{2,2,false},a.model().revision(),a.model().session()}));
            yes(a.move({100,1,{0,0,false},a.model().revision(),a.model().session()}));
            StorageViewOptions options;options.owner=100;options.center=true;NativeStorageView view;yes(view.build(a.model(),options));
            Id owner=0;Position local;check(view.destination(3,4,owner,local)&&owner==100&&local.x==3&&local.y==3,"destination mapping");
            DragSession drag;yes(drag.begin(a.model(),102));const auto before=a.model().serialize();
            auto preview=drag.preview(a.model(),owner,local.x,local.y,false,{});yes(preview.result);
            check(a.model().serialize()==before,"preview mutated authoritative placement");
            auto proposal=drag.release(a.model(),owner,local.x,local.y,false,{});yes(proposal.result);yes(a.move(proposal.moves.front()));
            check(a.model().nodes().at(102).parent==100&&a.model().nodes().at(102).position.x==3&&a.model().carried_mass()==1300,"cross-window commit owner or mass");
            yes(drag.begin(a.model(),102));auto invalid=drag.preview(a.model(),100,1,1,false,{});check(!invalid.result,"occupied cell accepted");
        });
        test("separate and collapsed storage views preserve model and hide inaccessible display cells",[]{
            auto bag=item(100,1,800);bag.container=true;bag.grid={8,8};bag.empty=bag.half=bag.full={2,2};
            auto inner=item(101,1,300);inner.container=true;inner.grid={4,4};
            auto med=item(102,1,200);
            ActorLayout a;yes(a.reconcile({bag,inner,med},{{4,2},{},true}));
            yes(a.move({101,100,{1,1,false},a.model().revision(),a.model().session()}));
            yes(a.move({102,101,{2,2,false},a.model().revision(),a.model().session()}));
            yes(a.move({100,1,{0,0,false},a.model().revision(),a.model().session()}));
            auto before=a.model().serialize();StorageViewOptions options;options.equipped_only=true;options.center=true;
            NativeStorageView main;yes(main.build(a.model(),options));
            check(main.bands.size()==1&&main.bands[0].column==2,"carried container leaked into equipped storage");
            Position p;check(!main.position(a.model().nodes().at(101),p)&&!main.position(a.model().nodes().at(102),p),"closed nested contents visible");
            check(main.recovery.empty()&&main.size.h==3,"empty recovery reserves a row");
            options.owner=101;NativeStorageView window;yes(window.build(a.model(),options));
            check(window.size.w==4&&window.position(a.model().nodes().at(102),p)&&p.x==2&&p.y==3,"separate window projection uses its actual layout extent");
            Id owner=0;Position local;check(window.destination(p.x,p.y,owner,local)&&owner==101&&local.x==2&&local.y==2,"window drop changed local coordinates");
            options.collapsed.insert(101);yes(window.build(a.model(),options));
            check(!window.position(a.model().nodes().at(102),p)&&!window.destination(4,1,owner,local),"collapsed grid accepts items");
            check(window.size.h==1&&window.bands.size()==1,"collapsed header missing or reserves grid");
            options.owner=999;check(!window.build(a.model(),options),"missing window owner accepted");
            check(a.model().serialize()==before,"view change mutated inventory");
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
        test("native ground pickup reserves last slot and preserves rejected source",[]{
            ActorLayout layout;ActorStorage storage{{1,1},{},true};yes(layout.reconcile({},storage));
            EntityRegistry registry;yes(registry.add(1));yes(registry.bind(1,7));
            yes(registry.add(100));yes(registry.bind(100,40));yes(registry.add(101));yes(registry.bind(101,41));
            auto incoming=item(100,2,38);incoming.attributes["native_section"]="bandage";
            GroundPickup first(registry);const auto before=layout.model().serialize();
            yes(first.prepare(layout.model(),incoming,{1}));check(!first.can_save(),"pending acquisition allowed save");
            check(layout.model().serialize()==before,"reservation changed live actor before acknowledgement");
            yes(first.acknowledge(7,true));yes(layout.restore(first.model().serialize()));
            check(layout.model().nodes().at(100).location==Location::Grid&&layout.model().carried_mass()==38,"accepted pickup used recovery or counted mass twice");
            GroundPickup second(registry);incoming.id=101;const auto full=layout.model().serialize();
            no(second.prepare(layout.model(),incoming,{1}),"no_fit");
            check(layout.model().serialize()==full,"last slot competing pickup changed actor");
            ActorLayout empty;yes(empty.reconcile({},storage));GroundPickup rejected(registry);incoming.id=100;
            yes(rejected.prepare(empty.model(),incoming,{1}));no(rejected.acknowledge(0xffff,false),"native_rejected");
            check(rejected.can_save()&&rejected.model().nodes().at(100).parent==2,"native refusal lost source");
        });
        test("server acknowledged arrivals reserve cells through client facts reconciliation",[]{
            ActorLayout layout;ActorStorage storage{{2,1},{},true};yes(layout.reconcile({},storage));
            EntityRegistry registry;yes(registry.add(1));yes(registry.bind(1,1));
            std::vector<Node> pending;
            for(Id id=100;id<102;++id) {
                yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id)));
                auto incoming=item(id,2,38);incoming.attributes["native_section"]="bandage";incoming.attributes["native_place"]="3";
                GroundPickup pickup(registry);yes(pickup.prepare(layout.model(),incoming,{1}));yes(pickup.acknowledge(1,true));
                yes(layout.restore(pickup.model().serialize()));
                auto fact=layout.model().nodes().at(id);fact.parent=1;fact.location=Location::Overflow;fact.position={};pending.push_back(fact);
                yes(layout.reconcile(pending,storage));
                check(layout.model().nodes().at(id).location==Location::Grid&&layout.model().nodes().at(id).position.x==static_cast<int>(id-100),"pending server item lost its reserved cell");
            }
            yes(registry.add(102));yes(registry.bind(102,102));auto extra=item(102,2,38);extra.attributes["native_section"]="bandage";
            GroundPickup blocked(registry);no(blocked.prepare(layout.model(),extra,{1}),"no_fit");
            check(layout.model().carried_mass()==76,"pending/client reconciliation duplicated mass");
            ActorLayout restarted;yes(restarted.restore(layout.model().serialize()));yes(restarted.reconcile(pending,storage));
            check(restarted.model().nodes().at(100).position.x==0&&restarted.model().nodes().at(101).position.x==1,"accepted positions lost on restart");
        });
        test("native ground pickup nested target and disappearing source fail closed",[]{
            ActorLayout layout;ActorStorage storage{{1,1},{},true};auto pack=bag(100,1,820);pack.soft=false;pack.empty=pack.half=pack.full={1,1};pack.attributes["native_section"]="itm_backpack";
            yes(layout.reconcile({pack},storage));yes(layout.move({100,1,{0,0,false},layout.model().revision(),layout.model().session()}));
            EntityRegistry registry;for(Id id:{Id{1},Id{100},Id{101}}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id+10)));}
            auto incoming=item(101,2,500);incoming.attributes["native_section"]="medkit";
            GroundPickup nested(registry);yes(nested.prepare(layout.model(),incoming,{1,100}));yes(nested.acknowledge(11,true));
            check(nested.model().nodes().at(101).parent==100&&nested.model().carried_mass()==1320,"nested pickup flattened or duplicated weight");
            GroundPickup vanished(registry);yes(vanished.prepare(layout.model(),incoming,{1,100}));yes(registry.unbind(*registry.handle(101)));
            no(vanished.acknowledge(11,true),"stale_entity");check(!vanished.can_save(),"disappeared source acknowledgement allowed save");
        });
        test("owned source admission refuses before detach and rolls back native rejection",[]{
            ActorLayout layout;ActorStorage storage{{1,1},{},true};yes(layout.reconcile({},storage));
            EntityRegistry registry;for(Id id:{Id{1},Id{2},Id{100}}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id+10)));}
            auto incoming=item(100,2,38);incoming.attributes["native_section"]="bandage";
            GroundPickup rejected(registry);yes(rejected.prepare(layout.model(),incoming,{1},false));
            no(rejected.acknowledge(12,false),"native_rejected");check(rejected.can_save()&&rejected.model().nodes().at(100).parent==2,"owned refusal lost source or blocked saves");
            GroundPickup accepted(registry);yes(accepted.prepare(layout.model(),incoming,{1},false));yes(accepted.acknowledge(11,true));
            yes(layout.restore(accepted.model().serialize()));
            yes(registry.add(101));yes(registry.bind(101,111));incoming.id=101;
            const auto full=layout.model().serialize();GroundPickup overflow(registry);no(overflow.prepare(layout.model(),incoming,{1},false),"no_fit");
            check(layout.model().serialize()==full,"owned capacity refusal changed carried state");
        });
        test("owned source disappearing after reservation requires reconciliation",[]{
            ActorLayout layout;yes(layout.reconcile({},{{1,1},{},true}));
            EntityRegistry registry;for(Id id:{Id{1},Id{2},Id{100}}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id+10)));}
            auto incoming=item(100,2,38);incoming.attributes["native_section"]="bandage";
            GroundPickup pending(registry);yes(pending.prepare(layout.model(),incoming,{1},false));
            yes(registry.unbind(*registry.handle(2)));no(pending.acknowledge(11,true),"stale_entity");check(!pending.can_save(),"vanished owned source allowed save");
        });
        test("owned direct equip bypasses full grid and reserves the empty equipment slot",[]{
            ActorLayout layout;auto existing=item(100,1,38);existing.attributes["native_section"]="bandage";
            yes(layout.reconcile({existing},{{1,1},{},true}));
            EntityRegistry registry;for(Id id:{Id{1},Id{2},Id{100},Id{101},Id{102}}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id+10)));}
            auto incoming=item(101,2,810);incoming.wear_slots=4;incoming.empty=incoming.half=incoming.full={2,2};
            const auto before=layout.model().serialize();GroundPickup bag_attempt(registry);
            no(bag_attempt.prepare(layout.model(),incoming,{1},false),"no_fit");
            GroundPickup equip(registry);yes(equip.prepare(layout.model(),incoming,{},false,4));
            check(!equip.can_save()&&layout.model().serialize()==before,"pending equip changed live actor");
            yes(equip.acknowledge(11,true));yes(layout.restore(equip.model().serialize()));
            check(layout.model().nodes().at(101).location==Location::Equipment&&layout.model().nodes().at(101).equipped_slots==4,"direct equip occupied bag capacity");
            check(layout.model().carried_mass()==848,"direct equip mass mismatch");
            incoming.id=102;GroundPickup occupied(registry);
            const auto full=layout.model().serialize();check(!occupied.prepare(layout.model(),incoming,{},false,4),"second item stole reserved slot");
            check(layout.model().serialize()==full,"occupied slot refusal mutated actor");
        });
        test("direct equip native refusal retains source and invalid slot cannot dispatch",[]{
            ActorLayout layout;yes(layout.reconcile({},{{1,1},{},true}));
            EntityRegistry registry;for(Id id:{Id{1},Id{2},Id{100}}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id+10)));}
            auto incoming=item(100,2,810);incoming.wear_slots=4;
            GroundPickup invalid(registry);check(!invalid.prepare(layout.model(),incoming,{},false,8),"unsupported slot admitted");
            GroundPickup refused(registry);yes(refused.prepare(layout.model(),incoming,{},false,4));
            no(refused.acknowledge(12,false),"native_rejected");
            check(refused.can_save()&&refused.model().nodes().at(100).parent==2&&refused.model().nodes().at(100).location==Location::Grid,"native equip refusal lost source");
        });
        test("purchase reserves whole batch and publishes only after ownership and money",[]{
            ActorLayout layout;yes(layout.reconcile({},{{2,1},{},true}));
            EntityRegistry registry;for(Id id:{Id{1},Id{2},Id{100},Id{101}}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id+10)));}
            PurchaseSession purchase(layout.model(),registry);const auto before=layout.model().serialize();
            yes(purchase.prepare({{item(100,2,38),7},{item(101,2,38),9}},{1},{20,100}));
            check(!purchase.can_save()&&!purchase.committed_model(),"prepared purchase published");
            yes(purchase.dispatch(true,{20,100}));
            yes(purchase.acknowledge({{100,11},{101,11}},true,{20,100}));
            check(!purchase.can_save()&&!purchase.committed_model(),"unsettled purchase published");
            check(purchase.settlement().actor==4&&purchase.settlement().seller==116,"incorrect settlement");
            yes(purchase.settled({4,116}));check(purchase.can_save()&&purchase.committed_model(),"settled purchase unavailable");
            check(purchase.committed_model()->carried_mass()==76,"purchase lost items");
            check(layout.model().serialize()==before,"purchase mutated live model before publication");
        });
        test("purchase whole-batch no-fit and insufficient money leave live state unchanged",[]{
            ActorLayout layout;yes(layout.reconcile({},{{1,1},{},true}));
            EntityRegistry registry;for(Id id:{Id{1},Id{2},Id{100},Id{101}}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id+10)));}
            const auto before=layout.model().serialize();PurchaseSession purchase(layout.model(),registry);
            no(purchase.prepare({{item(100,2,38),1},{item(101,2,38),1}},{1},{20,100}),"no_fit");
            check(purchase.can_save()&&!purchase.committed_model(),"failed batch reserved state");
            no(purchase.prepare({{item(100,2,38),21}},{1},{20,100}),"insufficient_money");
            check(layout.model().serialize()==before,"failed purchase mutated layout");
        });
        test("purchase access money and identity are rechecked before dispatch",[]{
            ActorLayout layout;yes(layout.reconcile({},{{1,1},{},true}));
            EntityRegistry registry;for(Id id:{Id{1},Id{2},Id{100}}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id+10)));}
            PurchaseSession purchase(layout.model(),registry);yes(purchase.prepare({{item(100,2,38),1}},{1},{20,100}));
            no(purchase.dispatch(false,{20,100}),"source_access");no(purchase.dispatch(true,{19,100}),"money_changed");
            yes(registry.unbind(*registry.handle(100)));yes(registry.bind(100,110));
            no(purchase.dispatch(true,{20,100}),"stale_entity");yes(purchase.cancel());check(purchase.can_save(),"cancel held save lock");
        });
        test("purchase partial native outcome and wrong settlement require reconciliation",[]{
            ActorLayout layout;yes(layout.reconcile({},{{2,1},{},true}));
            EntityRegistry registry;for(Id id:{Id{1},Id{2},Id{100},Id{101}}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id+10)));}
            PurchaseSession partial(layout.model(),registry);yes(partial.prepare({{item(100,2,38),1},{item(101,2,38),1}},{1},{20,100}));yes(partial.dispatch(true,{20,100}));
            no(partial.acknowledge({{100,11},{101,12}},true,{20,100}),"purchase_reconcile");check(!partial.can_save()&&!partial.committed_model(),"partial purchase published");
            PurchaseSession unsettled(layout.model(),registry);yes(unsettled.prepare({{item(100,2,38),1}},{1},{20,100}));yes(unsettled.dispatch(true,{20,100}));
            yes(unsettled.acknowledge({{100,11}},true,{20,100}));no(unsettled.settled({18,101}),"settlement_reconcile");check(!unsettled.can_save(),"wrong money allowed save");
        });
        test("purchase rejects noncarried destinations and overflowing seller balance",[]{
            ActorLayout layout;yes(layout.reconcile({},{{1,1},{},true}));
            EntityRegistry registry;for(Id id:{Id{1},Id{2},Id{100}}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id+10)));}
            PurchaseSession purchase(layout.model(),registry);
            no(purchase.prepare({{item(100,2,38),1}},{2},{20,100}),"purchase_destination");
            no(purchase.prepare({{item(100,2,38),1}},{1},{20,std::numeric_limits<std::uint32_t>::max()}),"money_overflow");
            check(purchase.can_save()&&!purchase.committed_model(),"invalid purchase held state");
        });
        test("purchase native rejection retains source and never settles money",[]{
            ActorLayout layout;yes(layout.reconcile({},{{1,1},{},true}));
            EntityRegistry registry;for(Id id:{Id{1},Id{2},Id{100}}){yes(registry.add(id));yes(registry.bind(id,static_cast<std::uint16_t>(id+10)));}
            PurchaseSession purchase(layout.model(),registry);yes(purchase.prepare({{item(100,2,38),1}},{1},{20,100}));yes(purchase.dispatch(true,{20,100}));
            no(purchase.acknowledge({{100,12}},false,{20,100}),"native_rejected");check(purchase.can_save()&&!purchase.committed_model(),"rejected purchase published");
            no(purchase.settled({19,101}),"purchase_state");
        });
        test("equipment registry reserves every vanilla u16 ID and rejects registration collisions",[]{
            EquipmentRegistry registry;EquipmentSlotDefinition slot;slot.key="sqa3.rig";slot.families="rig";slot.model_bit=19;
            for(unsigned id=0;id<=65535;++id){slot.id=id;no(registry.add(slot),"reserved_equipment_slot");}
            slot.id=65536;yes(registry.add(slot));
            no(registry.add(slot),"equipment_slot_collision");
            slot.id=65537;slot.key="sqa3.other";no(registry.add(slot),"equipment_slot_collision");
            slot.model_bit=20;yes(registry.add(slot));
            check(registry.find(65536)&&registry.find("sqa3.rig")->id==65536,"stable equipment lookup changed");
            slot.id=65538;slot.key="sqa3.mount.p1";slot.families="pouch";slot.mounted=true;slot.model_bit=0;slot.minimum_condition=1000;yes(registry.add(slot));
            check(registry.find(65538)->accepts("pouch")&&!registry.find(65538)->accepts("rig"),"equipment family gate failed");
        });
        test("native custom equipment facts preserve mounted contents and recover inactive grants",[&]{
            Node rig=item(100,1,900);rig.container=true;rig.grid={3,3};rig.empty=rig.half=rig.full={2,3};
            rig.location=Location::Equipment;rig.wear_slots=rig.equipped_slots=std::uint64_t{1}<<19;
            rig.mounts.push_back({65537,"custom_mount_65537",{1,1},1,1000,true});
            rig.attributes={{"native_section","test_rig"},{"native_place","4"},{"native_custom_slot","65536"},{"native_custom_host","1"},{"storage_allow_container_items","0"}};
            Node pouch=item(101,100,200);pouch.pouch=pouch.container=true;pouch.grid={5,1};pouch.location=Location::Mounted;pouch.mounted_slot=65537;
            pouch.attributes={{"native_section","test_pouch"},{"native_place","4"},{"native_custom_slot","65537"},{"native_custom_host","100"},{"storage_allow_container_items","0"}};
            auto med=item(102,1,500);med.attributes["native_section"]="medkit";
            ActorStorage pockets{{4,2},{},true};ActorLayout layout;yes(layout.reconcile({rig,pouch,med},pockets));
            yes(layout.move({102,101,{2,0,false},layout.model().revision(),layout.model().session()}));
            yes(save_atomic(layout.model(),std::string(argv[1])+".custom"));
            med.condition=8432;yes(layout.reconcile({rig,pouch,med},pockets));
            check(layout.model().nodes().at(102).parent==101&&layout.model().nodes().at(101).location==Location::Mounted,"native rebuild detached pouch contents");
            StorageViewOptions options;options.owner=100;options.center=true;NativeStorageView view;yes(view.build(layout.model(),options));
            check(view.bands.size()==2&&view.size.w==5&&view.bands[0].column==1&&view.bands[1].owner==101,"effective rig/pouch layout omitted or displaced a band");
            rig.condition=500;yes(layout.reconcile({rig,pouch,med},pockets));
            check(layout.model().nodes().at(101).location==Location::Mounted&&layout.model().nodes().at(102).location==Location::Overflow,"condition loss lost binding or failed recovery");
            check(layout.model().carried_mass()==1600&&layout.model().nodes().at(102).condition==8432,"condition recovery changed item state or mass");
        });
        test("quick access distinguishes worn rig and active pouch from possession and closed storage",[]{
            auto actor=root(1,Kind::ActorRoot);actor.allowed_slots=~std::uint64_t{0};actor.equipment_accepts=~std::uint64_t{0};actor.equipment_capacity=64;
            auto rig=bag(100,1,900);rig.attributes["storage_family"]="rig";rig.location=Location::Equipment;
            rig.wear_slots=rig.equipped_slots=std::uint64_t{1}<<19;
            rig.attributes["storage_exterior_independent"]="1";
            rig.mounts.push_back({65537,"left",{1,1},1,1000,true});
            auto pouch=bag(101,100,200);pouch.pouch=true;pouch.location=Location::Mounted;pouch.mounted_slot=65537;
            auto spare=bag(102,1,700,{4,0,false});spare.attributes["storage_family"]="rig";
            auto box=bag(103,1,100,{8,0,false});
            auto recovery=item(108,1,10);recovery.location=Location::Overflow;
            Model model;yes(model.initialize({actor,rig,pouch,spare,box,item(104,100,20),item(105,101,20),item(106,102,20),item(107,103,20),recovery,item(109,1,20,{12,0,false})}));
            check(equipped_rig_access(model,104)&&equipped_rig_access(model,105),"worn rig/pouch inaccessible");
            for(Id id:{Id{106},Id{107},Id{108},Id{109},Id{999}})check(!equipped_rig_access(model,id),"possession leaked into quick access");
            check(model.nodes().count(106)&&model.nodes().count(107),"access changed possession");
        });
        test("catalog applies authored storage and preserves native instance state",[]{
            StorageCatalog catalog;StorageDefinition pack;
            pack.family="backpack";pack.interior={{3,2},{1,1,0,2,2,0}};
            pack.empty={1,1};pack.half={2,1};pack.full={2,2};pack.growing=true;
            catalog.definitions.emplace("test_pack",pack);
            auto n=item(100,1,820);n.condition=7834;n.attributes["upgrade"]="retained";
            check(catalog.apply("test_pack",n),"catalog definition missing");
            check(n.container&&n.soft&&n.cells==pack.interior.cells&&n.grid.w==3,"catalog shape lost");
            check(n.intrinsic==820&&n.condition==7834&&n.attributes.at("upgrade")=="retained","catalog overwrote native state");
            check(n.attributes.at("storage_allow_container_items")=="0","nesting default not explicit");
            auto before=n.attributes;check(!catalog.apply("unknown",n)&&n.attributes==before,"unknown definition changed state");
        });
        test("catalog soft exterior is independent of usable interior and rig tears preserve authored restoration",[]{
            StorageCatalog catalog;StorageDefinition definition;
            definition.family="rig";definition.interior={{4,2},{1,1,1,2,3,3,0,2}};
            definition.empty={1,1};definition.half={1,2};definition.full={2,2};
            definition.growing=true;definition.independent_exterior=true;definition.tear_thresholds={8000,6000,4000,2500,1000};
            catalog.definitions.emplace("rig",definition);
            auto rig=item(100,1,500);check(catalog.apply("rig",rig),"missing catalog rig");
            Model intact;yes(intact.initialize({root(1,Kind::ActorRoot),rig}));
            rig.condition=8000;check(catalog.apply("rig",rig),"missing damaged definition");
            check(rig.cells==std::vector<unsigned>({0,0,0,2,3,3,0,2}),"largest pocket tear mismatch");
            rig.condition=6000;check(catalog.apply("rig",rig),"missing second tear definition");
            check(rig.cells==std::vector<unsigned>({0,0,0,2,0,0,0,2}),"tie must tear later authored pocket");
            rig.condition=0;check(catalog.apply("rig",rig),"missing destroyed definition");
            Model destroyed;yes(destroyed.initialize({root(1,Kind::ActorRoot),rig}));
            rig.condition=10000;check(catalog.apply("rig",rig),"missing repaired definition");
            check(rig.cells==definition.interior.cells&&rig.intrinsic==500,"repair failed authored restoration or changed mass");
        });
        test("authored backpack mask preserves compartment boundaries and rejects malformed imports atomically",[]{
            ActorStorage storage;yes(parse_storage_shape("3x4",7,storage));
            yes(parse_storage_mask("1,1,0,2|1,1,0,2|0,0,3,3",storage));
            check(storage.cells==std::vector<unsigned>({1,1,0,2,1,1,0,2,0,0,3,3}),"mask changed authored geometry");
            const auto before=storage.cells;
            for(const auto& malformed:{"1,1|1,1","1,1,0,2|1,1,0,2|0,0,3,","1,1,0,2|1,1,0,2|0,0,-1,3","0,0,0,0|0,0,0,0|0,0,0,0"}) {
                no(parse_storage_mask(malformed,storage),"storage_mask");check(storage.cells==before,"failed mask import changed layout");
            }
            Node actor=root(1,Kind::ActorRoot,storage.grid);actor.cells=storage.cells;
            Node wide=item(10,1,1);wide.empty=wide.half=wide.full={2,1};wide.position={2,2,false};
            Model m;yes(m.initialize({actor,wide}));
            rollback(m,{10,1,{0,2,false},m.revision()},"blocked_cell");
        });
        test("configurable portable-container nesting refuses new relationships and preserves legacy state",[]{
            auto nodes=fixture();nodes[2].attributes["storage_allow_container_items"]="0";
            Model m;yes(m.initialize(nodes));
            check(m.nodes().at(11).parent==10,"legacy nested bag lost");
            yes(move(m,{11,1,{4,4,false},m.revision()}));
            rollback(m,{11,10,{0,0,false},m.revision()},"container_nesting_policy");
            Model disk;yes(disk.deserialize(m.serialize()));check(disk.serialize()==m.serialize(),"policy persistence");
            auto before=m.serialize();
            no(m.relocate({{11,10,{0,0,false},m.revision(),m.session()}}),"container_nesting_policy");
            check(before==m.serialize(),"rejected compound admission mutated state");
            Node spawned=bag(30,10,10);Command spawn;spawn.operation=Operation::Spawn;spawn.spawned=spawned;spawn.revision=m.revision();spawn.session=m.session();
            no(m.execute(spawn),"container_nesting_policy");check(before==m.serialize(),"rejected spawn mutated state");
            nodes[2].attributes["storage_allow_container_items"]="1";
            Model enabled;yes(enabled.initialize(nodes));yes(move(enabled,{11,1,{4,4,false},enabled.revision()}));
            yes(move(enabled,{11,10,{0,0,false},enabled.revision()}));
            auto permitted=fixture();permitted[0].attributes["storage_allow_container_items"]="0";
            Model actor;yes(actor.initialize(permitted));yes(move(actor,{11,1,{4,4,false},actor.revision()}));
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
