#include "drag_session.h"
#include <algorithm>

namespace inventory_next {
bool DragSession::current(const Model& m) const {
    return item_ && session_==m.session() && revision_==m.revision() && m.nodes().count(item_);
}
Result DragSession::begin(const Model& m,Id id) {
    cancel();
    const auto found=m.nodes().find(id);
    if(found==m.nodes().end() || found->second.kind!=Kind::Item) return {false,"item"};
    item_=id; session_=m.session(); revision_=m.revision(); rotated_=found->second.position.rotated;
    return {true,{}};
}
void DragSession::cancel() { item_=0; session_=0; revision_=0; rotated_=false; }
bool DragSession::rotate(const Model& m) {
    if(!current(m)) { cancel(); return false; }
    rotated_=!rotated_; return true;
}
DropProposal DragSession::preview(const Model& m,Id destination,int x,int y,bool swap,const std::set<Id>& allowed) const {
    DropProposal p;
    if(!item_) return p;
    p.feedback=DropFeedback::Invalid;
    if(!current(m)) { p.result={false,"stale_drag"}; return p; }
    const auto& source=m.nodes().at(item_);
    p.position={x,y,rotated_}; p.footprint=m.footprint(item_);
    if(rotated_!=source.position.rotated) std::swap(p.footprint.w,p.footprint.h);
    // UI coordinates are untrusted. Avoid overflow even before core validation.
    if(x<0||y<0||x>64||y>64) { p.result={false,"position"}; return p; }
    Move incoming{item_,destination,p.position,revision_,session_};
    Model candidate=m;
    p.result=candidate.move(incoming);
    if(p.result) { p.feedback=DropFeedback::Valid; p.moves={incoming}; return p; }
    if(!swap||source.location!=Location::Grid) return p;
    // Grid swaps only. Equipment displacement requires a separate slot policy.
    Id target=0;
    for(const auto& pair:m.nodes()) {
        const auto& n=pair.second;
        if(n.id==item_||n.parent!=destination||n.location!=Location::Grid) continue;
        const auto size=m.footprint(n.id);
        if(x<n.position.x+size.w&&n.position.x<x+p.footprint.w&&
           y<n.position.y+size.h&&n.position.y<y+p.footprint.h) {
            if(target) { p.result={false,"multiple_swap_targets"}; return p; }
            target=n.id;
        }
    }
    if(!target) return p;
    if(!allowed.count(target)) { p.result={false,"swap_target_unavailable"}; return p; }
    Position back=source.position;
    back.rotated=m.nodes().at(target).position.rotated;
    std::vector<Move> moves{incoming,{target,source.parent,back,revision_,session_}};
    candidate=m;
    p.result=candidate.relocate(moves);
    if(p.result) { p.feedback=DropFeedback::Swap; p.moves=std::move(moves); }
    return p;
}
DropProposal DragSession::release(const Model& m,Id destination,int x,int y,bool swap,const std::set<Id>& allowed) {
    auto result=preview(m,destination,x,y,swap,allowed);
    cancel(); return result;
}
} // namespace inventory_next
