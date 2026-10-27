#include "model.h"
#include <set>
#include <limits>
namespace inventory_next {
Result Model::retry_recovery(Id actor,const std::vector<Id>& destinations,
                            std::uint64_t revision,std::uint64_t session) {
    if(!session_||session!=session_)return {false,"stale_session"};
    if(revision!=revision_)return {false,"stale_revision"};
    if(revision_==std::numeric_limits<std::uint64_t>::max())return {false,"revision_overflow"};
    auto owner=nodes_.find(actor);
    if(owner==nodes_.end()||owner->second.kind!=Kind::ActorRoot)return {false,"recovery_ownership"};
    if(destinations.empty()||destinations.size()>64)return {false,"recovery_destinations"};
    std::set<Id> unique;
    for(Id id:destinations) {
        auto n=nodes_.find(id);
        if(n==nodes_.end()||!n->second.container||!unique.insert(id).second)return {false,"recovery_destinations"};
        Id root=id;for(unsigned depth=0;nodes_.at(root).parent&&depth<=64;++depth)root=nodes_.at(root).parent;
        if(root!=actor)return {false,"recovery_ownership"};
    }
    Model candidate=*this;unsigned budget=8192;bool changed=false;
    for(const auto& [id,n]:nodes_)if(n.parent==actor&&n.location==Location::Overflow&&budget) {
        bool placed=false;
        for(Id destination:destinations) {
            if(placed||!budget)break;
            if(!candidate.storage_active(destination))continue;
            const Size grid=candidate.nodes().at(destination).grid;
            for(unsigned rotation=0;rotation<2&&!placed&&budget;++rotation)
                for(int y=0;y<grid.h&&!placed&&budget;++y)
                    for(int x=0;x<grid.w&&!placed&&budget;++x) {
                        --budget;
                        placed=candidate.move({id,destination,{x,y,rotation!=0},candidate.revision(),session}).ok;
                    }
        }
        changed=changed||placed;
    }
    if(changed){candidate.revision_=revision_+1;*this=std::move(candidate);}
    return {true,{}};
}
Result Model::recover_recipe(const std::vector<Id>& consumed,const std::vector<Node>& outputs,
                             Id actor,std::uint64_t revision,std::uint64_t session) {
    if(!session_||session!=session_)return {false,"stale_session"};
    if(revision!=revision_)return {false,"stale_revision"};
    if(consumed.empty()||outputs.empty()||consumed.size()>64||outputs.size()>64)return {false,"recipe_size"};
    const auto owner=nodes_.find(actor);if(owner==nodes_.end()||owner->second.kind!=Kind::ActorRoot)return {false,"recovery_ownership"};
    Model candidate=*this;std::set<Id> unique;
    for(Id id:consumed) {
        auto found=candidate.nodes().find(id);if(found==candidate.nodes().end()||found->second.kind!=Kind::Item||!unique.insert(id).second)return {false,"recipe_input"};
        Id root=id;for(unsigned depth=0;candidate.nodes().at(root).parent&&depth<=64;++depth)root=candidate.nodes().at(root).parent;
        if(root!=actor)return {false,"recovery_ownership"};
        Command erase;erase.operation=Operation::Erase;erase.item=id;erase.revision=candidate.revision();erase.session=session;
        auto result=candidate.execute(erase);if(!result)return result;
    }
    unsigned budget=8192;
    for(auto item:outputs) {
        if(item.parent!=actor||item.kind!=Kind::Item)return {false,"recovery_ownership"};
        item.location=Location::Grid;item.equipped_slots=0;item.mounted_slot=0;
        Model validation=candidate;auto valid=validation.recover_native_arrival(item,validation.revision(),session);
        if(!valid)return valid;
        bool placed=false;
        for(unsigned rotation=0;rotation<2&&!placed&&budget;++rotation)
            for(int y=0;y<owner->second.grid.h&&!placed&&budget;++y)
                for(int x=0;x<owner->second.grid.w&&!placed&&budget;++x) {
                    --budget;item.position={x,y,rotation!=0};
                    Command spawn;spawn.operation=Operation::Spawn;spawn.spawned=item;spawn.session=session;spawn.revision=candidate.revision();
                    placed=candidate.execute(spawn).ok;
                }
        if(!placed) {
            auto result=candidate.recover_native_arrival(item,candidate.revision(),session);if(!result)return result;
        }
    }
    // Input deletion and every retained output are published together, once.
    candidate.revision_=revision_+1;*this=std::move(candidate);return {true,{}};
}
}
