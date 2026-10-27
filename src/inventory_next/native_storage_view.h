#pragma once
#include "model.h"
#include <algorithm>

namespace inventory_next {
// Presentation-only bands in the existing native scrolling item list. Grid
// coordinates remain local to their logical owner; headers/recovery are never
// legal drop destinations. No ownership, mass or item state lives in this view.
struct StorageBand { Id owner=0; int row=0; Size grid; };
class NativeStorageView {
public:
    Result build(const Model& model) {
        NativeStorageView next;
        auto actor=model.nodes().find(1);
        if(actor==model.nodes().end())return {false,"view_actor"};
        int row=1;
        auto add=[&](Id owner,const Size& size) {
            next.bands.push_back({owner,row,size});row+=size.h+1;
            next.size.w=std::max(next.size.w,size.w);
        };
        add(1,actor->second.grid);
        for(const auto& pair:model.nodes())if(pair.second.kind==Kind::Item&&pair.second.container)
            add(pair.first,pair.second.grid);
        next.recovery_row=row;
        for(const auto& pair:model.nodes())if(pair.second.location==Location::Overflow) {
            const auto footprint=model.footprint(pair.first);
            next.recovery[pair.first]={0,row,false};row+=footprint.h;
            next.size.w=std::max(next.size.w,footprint.w);
        }
        next.size.h=std::max(row, next.recovery_row+1);
        if(next.size.w>64||next.size.h>65536/next.size.w)return {false,"view_budget"};
        *this=std::move(next);return {true,{}};
    }
    bool position(const Node& item,Position& output) const {
        if(item.location==Location::Overflow) {
            auto found=recovery.find(item.id);if(found==recovery.end())return false;
            output=found->second;return true;
        }
        if(item.location!=Location::Grid)return false;
        for(const auto& band:bands)if(band.owner==item.parent) {
            output=item.position;output.y+=band.row;return true;
        }
        return false;
    }
    bool destination(int x,int y,Id& owner,Position& local) const {
        for(const auto& band:bands)if(x>=0&&x<band.grid.w&&y>=band.row&&y<band.row+band.grid.h) {
            owner=band.owner;local={x,y-band.row,false};return true;
        }
        return false;
    }
    Size size{8,1};
    int recovery_row=1;
    std::vector<StorageBand> bands;
    std::map<Id,Position> recovery;
};
}
