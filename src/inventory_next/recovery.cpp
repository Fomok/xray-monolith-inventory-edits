#include "model.h"
#include <set>
namespace inventory_next {
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
