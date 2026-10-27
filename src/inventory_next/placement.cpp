#include "placement.h"
#include <algorithm>
#include <set>
#include <tuple>

namespace inventory_next {
namespace {
bool same(Position a,Position b) { return a.x==b.x&&a.y==b.y&&a.rotated==b.rotated; }
Size oriented(const Model& m,Id id,bool rotation) {
    auto size=m.footprint(id);
    if(m.nodes().at(id).position.rotated!=rotation) std::swap(size.w,size.h);
    return size;
}
}
PlacementPlan find_placement(const Model& m,Id id,const std::vector<Id>& destinations,bool rotate,unsigned budget) {
    PlacementPlan plan;
    auto source=m.nodes().find(id);
    if(source==m.nodes().end()||source->second.kind!=Kind::Item) { plan.result={false,"item"}; return plan; }
    if(budget==0||budget>8192||destinations.size()>64) { plan.result={false,"search_limit"}; return plan; }
    std::set<Id> visited;
    for(auto target:destinations) {
        if(!visited.insert(target).second) continue;
        const auto found=m.nodes().find(target);
        if(found==m.nodes().end()||!found->second.container) continue;
        for(int rotation=0;rotation<(rotate?2:1);++rotation) {
            const bool r=source->second.position.rotated^(rotation!=0);
            const auto size=oriented(m,id,r);
            for(int y=0;y+size.h<=found->second.grid.h;++y) for(int x=0;x+size.w<=found->second.grid.w;++x) {
                if(plan.probes>=budget) { plan.result={false,"search_limit"}; return plan; }
                ++plan.probes;
                Move move{id,target,{x,y,r},m.revision(),m.session()};
                Model trial=m;
                if(trial.move(move)) { plan.result={true,{}}; plan.moves={move}; return plan; }
            }
        }
    }
    return plan;
}
PlacementPlan plan_stash_sort(const Model& m,Id stash,bool enabled,const std::map<Id,unsigned>& ranks,bool rotate) {
    PlacementPlan best;
    const auto found=m.nodes().find(stash);
    if(!enabled||found==m.nodes().end()||found->second.kind!=Kind::WorldRoot) { best.result={false,"not_stash"}; return best; }
    const auto grid=found->second.grid;
    std::vector<Id> children;
    int original_height=0;
    for(const auto& pair:m.nodes()) if(pair.second.parent==stash&&pair.second.location==Location::Grid) {
        children.push_back(pair.first);
        original_height=std::max(original_height,pair.second.position.y+m.footprint(pair.first).h);
    }
    if(children.size()>256) { best.result={false,"search_limit"}; return best; }
    // Existing placement is always the valid fallback. Sorting may not make it taller.
    best.result={true,{}};
    int best_height=original_height;
    auto rank=[&](Id id) { const auto r=ranks.find(id); return r==ranks.end()?~0u:r->second; };
    for(unsigned strategy=0;strategy<2;++strategy) {
        auto order=children;
        std::sort(order.begin(),order.end(),[&](Id a,Id b) {
            const auto as=m.footprint(a),bs=m.footprint(b);
            if(strategy==0) return std::make_tuple(rank(a),-as.w*as.h,a)<std::make_tuple(rank(b),-bs.w*bs.h,b);
            return std::make_tuple(-as.w*as.h,rank(a),a)<std::make_tuple(-bs.w*bs.h,rank(b),b);
        });
        std::vector<bool> occupied(static_cast<std::size_t>(grid.w*grid.h),false);
        std::vector<Move> moves;
        int height=0;
        bool complete=true;
        for(auto id:order) {
            bool placed=false;
            Position p;
            Size size;
            for(int y=0;y<grid.h&&!placed;++y) for(int x=0;x<grid.w&&!placed;++x)
                for(int rotation=0;rotation<(rotate?2:1)&&!placed;++rotation) {
                    ++best.probes;
                    p={x,y,m.nodes().at(id).position.rotated!=(rotation!=0)}; size=oriented(m,id,p.rotated);
                    if(x+size.w>grid.w||y+size.h>grid.h) continue;
                    const auto region=m.cell_region(stash,x,y);
                    if(!region) continue;
                    bool fits=true;
                    for(int dy=0;dy<size.h&&fits;++dy) for(int dx=0;dx<size.w;++dx)
                        if(occupied[static_cast<std::size_t>((y+dy)*grid.w+x+dx)]||m.cell_region(stash,x+dx,y+dy)!=region) { fits=false; break; }
                    if(fits) placed=true;
                }
            if(!placed) { complete=false; break; }
            for(int dy=0;dy<size.h;++dy) for(int dx=0;dx<size.w;++dx)
                occupied[static_cast<std::size_t>((p.y+dy)*grid.w+p.x+dx)]=true;
            height=std::max(height,p.y+size.h);
            if(!same(p,m.nodes().at(id).position)) moves.push_back({id,stash,p,m.revision(),m.session()});
        }
        if(!complete||height>best_height||moves.empty()) continue;
        Model trial=m;
        if(!trial.relocate(moves)) continue;
        if(height<best_height||best.moves.empty()) { best_height=height; best.moves=std::move(moves); }
    }
    return best;
}
} // namespace inventory_next
