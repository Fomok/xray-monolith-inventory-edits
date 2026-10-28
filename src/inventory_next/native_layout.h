#pragma once
#include "model.h"
#include <algorithm>
#include <limits>
namespace inventory_next {
// Bounded staging policy for the flat native projection, not the final logical
// pockets/backpack/rig forest. The native caller still supplies development size
// until admission, reservations and explicit recovery presentation are connected.
struct ActorStorage {
    Size grid{8,64};
    std::vector<unsigned> cells;
    // Explicit opt-in until native panels and admission share this forest.
    bool logical = false;
};
struct RuckAdmission {
    Result result{false,"capacity"};
    Position position;
};
// Actor-owned native item projection. Native objects retain their real physical
// owner; this slice owns placement only, not pickup/equipment/container authority.
class ActorLayout {
public:
    Result reconcile(std::vector<Node> live, const ActorStorage& storage = {}) {
        std::sort(live.begin(),live.end(),[](const Node& a,const Node& b){return a.id<b.id;});
        Node actor;actor.id=1;actor.kind=Kind::ActorRoot;actor.category=0;actor.container=true;actor.grid=storage.grid;actor.cells=storage.cells;actor.allowed_slots=~std::uint64_t{0};actor.equipment_accepts=~std::uint64_t{0};actor.equipment_capacity=64;
        if(storage.logical)actor.attributes["native_logical_storage"]="1";
        if(initialized()&&model_.nodes().at(1).attributes.count("native_logical_storage")&&!storage.logical)
            return {false,"native_layout_mode"};
        Node world;world.id=2;world.kind=Kind::WorldRoot;world.category=0;world.container=true;world.grid={8,64};
        std::vector<Node> factsNodes{actor,world};unsigned beltSlot=48;
        for(auto& n:live) {
            if(n.kind!=Kind::Item||n.id<100||(n.parent!=1&&n.location!=Location::Mounted)||(n.container&&!storage.logical))return {false,"native_layout_item"};
            if(n.attributes.count("native_place")&&n.attributes.at("native_place")=="2") {
                if(beltSlot>=64)return {false,"native_belt_capacity"};
                n.location=Location::Equipment;n.wear_slots=n.equipped_slots=std::uint64_t{1}<<beltSlot++;
            }
            if(n.location!=Location::Equipment&&n.location!=Location::Mounted)n.location=Location::Overflow;
            // Facts may come from an existing projection. Recovery/equipment
            // staging has no grid coordinates; saved placements are reapplied
            // from model_ only after the new native facts validate.
            n.position={};
            factsNodes.push_back(n);
        }
        Model facts;auto result=facts.initialize(factsNodes);if(!result)return result;
        const auto signature=facts.serialize();
        if(signature==signature_)return {true,{}};
        // Build from actual engine values, never import saved mass or item state.
        Model candidate=std::move(facts);
        // Restore inner contents before their shells. A loaded soft bag must
        // acquire its final footprint before its old outer placement is tested.
        auto depth=[&](Id id) {
            unsigned value=0;auto found=model_.nodes().find(id);
            while(found!=model_.nodes().end()&&found->second.parent&&value<=64) {
                ++value;found=model_.nodes().find(found->second.parent);
            }
            return value;
        };
        std::stable_sort(live.begin(),live.end(),[&](const Node& a,const Node& b){return depth(a.id)>depth(b.id);});
        for(const auto& n:live) {
            auto old=model_.nodes().find(n.id);
            if(old==model_.nodes().end())continue;
            auto section=old->second.attributes.find("native_section");
            auto current=n.attributes.find("native_section");
            if(section==old->second.attributes.end()||current==n.attributes.end()||section->second!=current->second)
                return {false,"native_identity_section"};
            if(n.location==Location::Equipment||old->second.location!=Location::Grid)continue;
            const Id destination=storage.logical?old->second.parent:1;
            // Missing/changed storage cannot silently reassign contents to pockets.
            // The native object remains actor-owned and visible in recovery.
            // Reapply only this exact prior identity/section/parent relationship.
            // New admission policy may forbid creating it, but cannot erase a
            // previously saved nested bag while rebuilding native facts.
            Command restore;restore.operation=Operation::Move;restore.item=n.id;
            restore.destination=destination;restore.position=old->second.position;
            restore.revision=candidate.revision();restore.session=candidate.session();
            candidate.execute_internal(restore,false,true);
            // Forced native size/state change may invalidate the prior position.
            // Preserve the actual item in explicit recovery until a valid fit exists.
        }
        if(!storage.logical) {
            result=candidate.retry_recovery(1,{1},candidate.revision(),candidate.session());if(!result)return result;
        }
        model_=std::move(candidate);signature_=signature;return {true,{}};
    }
    Result restore(const std::string& bytes) {
        Model candidate;auto result=candidate.deserialize(bytes);if(!result)return result;
        const auto a=candidate.nodes().find(1),w=candidate.nodes().find(2);
        if(a==candidate.nodes().end()||w==candidate.nodes().end()||a->second.kind!=Kind::ActorRoot||w->second.kind!=Kind::WorldRoot||
           a->second.max_contents)
            return {false,"native_layout_root"};
        for(const auto& pair:candidate.nodes())if(pair.second.kind!=Kind::Item&&pair.first!=1&&pair.first!=2)
            return {false,"native_layout_root"};
        const bool logical=a->second.attributes.count("native_logical_storage")!=0;
        if(logical&&a->second.attributes.at("native_logical_storage")!="1")return {false,"native_layout_mode"};
        for(const auto& pair:candidate.nodes())if(pair.second.kind==Kind::Item) {
            const auto& n=pair.second;
            if(n.id<100||(!logical&&(n.parent!=1||n.container))||
               (n.location!=Location::Grid&&n.location!=Location::Overflow&&n.location!=Location::Equipment&&n.location!=Location::Mounted))return {false,"native_layout_item"};
            Id owner=n.parent;
            for(unsigned count=0;candidate.nodes().at(owner).parent&&count<=64;++count)owner=candidate.nodes().at(owner).parent;
            if(owner!=1)return {false,"native_layout_owner"};
        }
        model_=std::move(candidate);signature_.clear();return {true,{}};
    }
    void reset(){model_=Model{};signature_.clear();}
    bool initialized() const{return model_.session()!=0;}
    const Model& model()const{return model_;}
    Result move(const Move& move){
        if(move.destination!=1&&(!initialized()||!model_.nodes().at(1).attributes.count("native_logical_storage")))return {false,"native_layout_owner"};
        const auto found=model_.nodes().find(move.destination);
        if(found==model_.nodes().end())return {false,"native_layout_owner"};
        Id owner=move.destination;
        for(unsigned count=0;model_.nodes().at(owner).parent&&count<=64;++count)owner=model_.nodes().at(owner).parent;
        if(owner!=1)return {false,"native_layout_owner"};
        return model_.move(move);
    }
    // Pure preflight: does not enroll an engine ID, move existing items, reserve
    // cells, or authorize a later asynchronous event. Commit must revalidate.
    RuckAdmission preview_ruck(Node incoming) const {
        if(!initialized())return {{false,"native_layout_uninitialized"},{}};
        if(model_.high_water_id()==std::numeric_limits<Id>::max())return {{false,"id_overflow"},{}};
        incoming.id=model_.high_water_id()+1;incoming.parent=1;
        incoming.location=Location::Grid;incoming.equipped_slots=0;incoming.mounted_slot=0;
        const Size grid=model_.nodes().at(1).grid;
        for(unsigned rotation=0;rotation<2;++rotation)
            for(int y=0;y<grid.h;++y)for(int x=0;x<grid.w;++x) {
                incoming.position={x,y,rotation!=0};
                Command spawn;spawn.operation=Operation::Spawn;spawn.spawned=incoming;
                spawn.session=model_.session();spawn.revision=model_.revision();
                auto result=model_.preview(spawn).result;
                if(result)return {result,incoming.position};
            }
        return {{false,"capacity"},{}};
    }
private:
    Model model_;
    std::string signature_;
};
}
