#pragma once
#include "model.h"
#include <algorithm>
namespace inventory_next {
// Actor-owned native item projection. Native objects retain their real physical
// owner; this slice owns placement only, not pickup/equipment/container authority.
class ActorLayout {
public:
    Result reconcile(std::vector<Node> live) {
        std::sort(live.begin(),live.end(),[](const Node& a,const Node& b){return a.id<b.id;});
        Node actor;actor.id=1;actor.kind=Kind::ActorRoot;actor.category=0;actor.container=true;actor.grid={16,64};
        Node world=actor;world.id=2;world.kind=Kind::WorldRoot;
        Model facts;auto result=facts.initialize({actor,world});if(!result)return result;
        for(auto& n:live) {
            if(n.kind!=Kind::Item||n.id<100||n.parent!=1||n.container)return {false,"native_layout_item"};
            result=facts.recover_native_arrival(n,facts.revision(),facts.session());if(!result)return result;
        }
        const auto signature=facts.serialize();
        if(signature==signature_)return {true,{}};
        // Build from actual engine values, never import saved mass or item state.
        Model candidate=std::move(facts);
        for(const auto& n:live) {
            auto old=model_.nodes().find(n.id);
            if(old==model_.nodes().end()||old->second.location!=Location::Grid)continue;
            auto section=old->second.attributes.find("native_section");
            auto current=n.attributes.find("native_section");
            if(section==old->second.attributes.end()||current==n.attributes.end()||section->second!=current->second)
                return {false,"native_identity_section"};
            candidate.move({n.id,1,old->second.position,candidate.revision(),candidate.session()});
            // Forced native size/state change may invalidate the prior position.
            // Preserve the actual item in explicit recovery until a valid fit exists.
        }
        result=candidate.retry_recovery(1,{1},candidate.revision(),candidate.session());if(!result)return result;
        model_=std::move(candidate);signature_=signature;return {true,{}};
    }
    Result restore(const std::string& bytes) {
        Model candidate;auto result=candidate.deserialize(bytes);if(!result)return result;
        const auto a=candidate.nodes().find(1),w=candidate.nodes().find(2);
        if(a==candidate.nodes().end()||w==candidate.nodes().end()||a->second.kind!=Kind::ActorRoot||w->second.kind!=Kind::WorldRoot||
           a->second.grid.w!=16||a->second.grid.h!=64||!a->second.cells.empty()||a->second.allowed_slots||a->second.max_contents)
            return {false,"native_layout_root"};
        for(const auto& pair:candidate.nodes())if(pair.second.kind!=Kind::Item&&pair.first!=1&&pair.first!=2)
            return {false,"native_layout_root"};
        for(const auto& pair:candidate.nodes())if(pair.second.kind==Kind::Item&&
            (pair.second.id<100||pair.second.parent!=1||pair.second.container||
             (pair.second.location!=Location::Grid&&pair.second.location!=Location::Overflow)))return {false,"native_layout_item"};
        model_=std::move(candidate);signature_.clear();return {true,{}};
    }
    void reset(){model_=Model{};signature_.clear();}
    bool initialized() const{return model_.session()!=0;}
    const Model& model()const{return model_;}
    Result move(const Move& move){if(move.destination!=1)return {false,"native_layout_owner"};return model_.move(move);}
private:
    Model model_;
    std::string signature_;
};
}
