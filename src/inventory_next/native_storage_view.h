#pragma once
#include "model.h"
#include <algorithm>
#include <set>

namespace inventory_next {
// Presentation-only bands in the existing native scrolling item list. Grid
// coordinates remain local to their logical owner; headers/recovery are never
// legal drop destinations. No ownership, mass or item state lives in this view.
struct StorageBand { Id owner=0; int row=0; Size grid; int column=0; bool collapsed=false; bool accepts_drops=true; };
struct StorageViewOptions {
    bool equipped_only=false;
    bool center=false;
    Id owner=0; // zero is the actor view; otherwise one explicitly opened container
    std::set<Id> collapsed;
};
class NativeStorageView {
public:
    Result build(const Model& model,const StorageViewOptions& options={}) {
        NativeStorageView next;
        if(options.owner)next.size.w=1;
        auto actor=model.nodes().find(1);
        if(actor==model.nodes().end())return {false,"view_actor"};
        if(options.owner) {
            const auto owner=model.nodes().find(options.owner);
            if(owner==model.nodes().end()||!owner->second.container||owner->second.kind!=Kind::Item)
                return {false,"view_owner"};
        }
        int row=1;
        auto add=[&](Id owner,const Size& size) {
            const bool collapsed=options.collapsed.count(owner)!=0;
            bool accepts=true;
            for(Id current=owner;current!=1;){
                const auto found=model.nodes().find(current);
                if(found==model.nodes().end()||found->second.location==Location::Overflow||!model.storage_active(current)){accepts=false;break;}
                current=found->second.parent;
            }
            next.bands.push_back({owner,row,size,0,collapsed,accepts});row+=(collapsed?0:size.h)+1;
            next.size.w=std::max(next.size.w,size.w);
        };
        if(options.owner)add(options.owner,model.nodes().at(options.owner).grid);
        else {
            add(1,actor->second.grid);
            for(const auto& pair:model.nodes())if(pair.second.kind==Kind::Item&&pair.second.container&&
                (!options.equipped_only||(pair.second.parent==1&&pair.second.location==Location::Equipment)))
                add(pair.first,pair.second.grid);
        }
        // Mounted pouch bands belong to the same effective layout as their rig.
        // Include them when that rig is visible, including a spare-rig window.
        std::set<Id> visible;
        for(const auto& band:next.bands)visible.insert(band.owner);
        for(const auto& pair:model.nodes())if(pair.second.container&&pair.second.location==Location::Mounted&&
            visible.count(pair.second.parent)&&!visible.count(pair.first))add(pair.first,pair.second.grid);
        next.recovery_row=row;
        for(const auto& pair:model.nodes())if(!options.owner&&pair.second.location==Location::Overflow) {
            const auto footprint=model.footprint(pair.first);
            next.recovery[pair.first]={0,row,false};row+=footprint.h;
            next.size.w=std::max(next.size.w,footprint.w);
        }
        next.size.h=next.recovery.empty()?std::max(1,row-1):row;
        if(options.center)for(auto& band:next.bands)band.column=(next.size.w-band.grid.w)/2;
        if(next.size.w>64||next.size.h>65536/next.size.w)return {false,"view_budget"};
        *this=std::move(next);return {true,{}};
    }
    bool position(const Node& item,Position& output) const {
        if(item.location==Location::Overflow) {
            auto found=recovery.find(item.id);if(found==recovery.end())return false;
            output=found->second;return true;
        }
        if(item.location!=Location::Grid)return false;
        for(const auto& band:bands)if(band.owner==item.parent&&!band.collapsed) {
            output=item.position;output.x+=band.column;output.y+=band.row;return true;
        }
        return false;
    }
    bool destination(int x,int y,Id& owner,Position& local) const {
        for(const auto& band:bands)if(!band.collapsed&&band.accepts_drops&&x>=band.column&&x<band.column+band.grid.w&&y>=band.row&&y<band.row+band.grid.h) {
            owner=band.owner;local={x-band.column,y-band.row,false};return true;
        }
        return false;
    }
    Size size{8,1};
    int recovery_row=1;
    std::vector<StorageBand> bands;
    std::map<Id,Position> recovery;
};
}
