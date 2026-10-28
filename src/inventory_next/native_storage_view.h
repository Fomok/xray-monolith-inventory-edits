#pragma once
#include "model.h"
#include <algorithm>
#include <map>
#include <set>

namespace inventory_next {
// Presentation-only bands in the existing native scrolling item list. Grid
// coordinates remain local to their logical owner; headers/recovery are never
// legal drop destinations. No ownership, mass or item state lives in this view.
struct StorageBand {
    Id owner=0; int row=0; Size grid; int column=0; bool collapsed=false; bool accepts_drops=true; bool header=true;
    std::map<std::pair<int,int>,std::pair<int,int>> projected;
    std::map<std::pair<int,int>,std::pair<int,int>> inverse;
    std::map<std::pair<int,int>,unsigned> regions;
};
struct StorageViewOptions {
    bool equipped_only=false;
    bool center=false;
    Id owner=0; // zero is the actor view; otherwise one explicitly opened container
    std::set<Id> collapsed;
};
class NativeStorageView {
public:
    Result build(const Model& model,const StorageViewOptions& options={}) {
        NativeStorageView next;next.centered=options.center;
        if(options.owner)next.size.w=1;
        auto actor=model.nodes().find(1);
        if(actor==model.nodes().end())return {false,"view_actor"};
        if(options.owner) {
            const auto owner=model.nodes().find(options.owner);
            if(owner==model.nodes().end()||!owner->second.container||owner->second.kind!=Kind::Item)
                return {false,"view_owner"};
        }
        int row=1;
        auto add=[&](Id owner,const Size& size,bool header=true,Id group=0)->Result {
            const bool collapsed=options.collapsed.count(group?group:owner)!=0;
            bool accepts=true;
            for(Id current=owner;current!=1;){
                const auto found=model.nodes().find(current);
                if(found==model.nodes().end()||found->second.location==Location::Overflow||!model.storage_active(current)){accepts=false;break;}
                current=found->second.parent;
            }
            StorageBand band;band.owner=owner;band.row=row;band.grid=size;band.collapsed=collapsed;band.accepts_drops=accepts;band.header=header;
            const auto& n=model.nodes().at(owner);
            struct Bounds {int left=64,top=64,right=-1,bottom=-1,dx=0,dy=0;};
            std::map<unsigned,Bounds> regions;
            for(int y=0;y<size.h;++y)for(int x=0;x<size.w;++x){
                const unsigned region=n.cells.empty()?1:n.cells[static_cast<std::size_t>(y*size.w+x)];
                if(!region)continue;
                auto& box=regions[region];box.left=std::min(box.left,x);box.top=std::min(box.top,y);
                box.right=std::max(box.right,x);box.bottom=std::max(box.bottom,y);
            }
            // Each authored region keeps its own cell spacing. A blank display
            // cell between touching rectangular regions is presentation only.
            for(std::size_t pass=0;pass<regions.size();++pass)for(auto& target:regions)for(const auto& source:regions)if(target.first!=source.first){
                const auto& a=source.second;auto& b=target.second;
                if(a.top<=b.bottom&&b.top<=a.bottom&&a.right<b.left)
                    b.dx=std::max(b.dx,a.dx+a.right+2-b.left);
                if(a.left<=b.right&&b.left<=a.right&&a.bottom<b.top)
                    b.dy=std::max(b.dy,a.dy+a.bottom+2-b.top);
            }
            for(int y=0;y<size.h;++y)for(int x=0;x<size.w;++x){
                const unsigned region=n.cells.empty()?1:n.cells[static_cast<std::size_t>(y*size.w+x)];
                if(!region)continue;
                const auto& box=regions.at(region);const std::pair<int,int> local{x,y},display{x+box.dx,y+box.dy};
                if(!band.inverse.emplace(display,local).second)return Result{false,"compartment_projection"};
                band.projected.emplace(local,display);band.regions.emplace(display,region);
                band.grid.w=std::max(band.grid.w,display.first+1);band.grid.h=std::max(band.grid.h,display.second+1);
            }
            next.bands.push_back(std::move(band));row+=(collapsed?0:next.bands.back().grid.h)+1;
            next.size.w=std::max(next.size.w,next.bands.back().grid.w);
            return {true,{}};
        };
        auto add_mounts=[&](Id rig)->Result {
            if(options.collapsed.count(rig))return {true,{}};
            for(const auto& pair:model.nodes())if(pair.second.container&&pair.second.location==Location::Mounted&&pair.second.parent==rig)
                if(const auto result=add(pair.first,pair.second.grid,false,rig);!result)return result;
            return {true,{}};
        };
        if(options.owner){
            if(const auto result=add(options.owner,model.nodes().at(options.owner).grid);!result)return result;
            if(const auto result=add_mounts(options.owner);!result)return result;
        }
        else {
            if(const auto result=add(1,actor->second.grid);!result)return result;
            for(const auto& pair:model.nodes())if(pair.second.kind==Kind::Item&&pair.second.container&&
                pair.second.location!=Location::Mounted&&
                (!options.equipped_only||(pair.second.parent==1&&pair.second.location==Location::Equipment))){
                if(const auto result=add(pair.first,pair.second.grid);!result)return result;
                if(const auto result=add_mounts(pair.first);!result)return result;
            }
        }
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
            const auto projected=band.projected.find({item.position.x,item.position.y});
            if(projected==band.projected.end())return false;
            output=item.position;output.x=projected->second.first+band.column;output.y=projected->second.second+band.row;return true;
        }
        return false;
    }
    bool destination(int x,int y,Id& owner,Position& local) const {
        for(const auto& band:bands)if(!band.collapsed&&band.accepts_drops){
            const auto found=band.inverse.find({x-band.column,y-band.row});
            if(found==band.inverse.end())continue;
            owner=band.owner;local={found->second.first,found->second.second,false};return true;
        }
        return false;
    }
    unsigned region(int x,int y) const {
        for(const auto& band:bands)if(!band.collapsed){
            const auto found=band.regions.find({x-band.column,y-band.row});
            if(found!=band.regions.end())return found->second;
        }
        return 0;
    }
    float row_inset(int y) const {
        if(centered)for(const auto& band:bands)if(!band.collapsed&&y>=band.row&&y<band.row+band.grid.h)
            return float((size.w-band.grid.w)%2)*0.5f;
        return 0.0f;
    }
    bool centered=false;
    Size size{8,1};
    int recovery_row=1;
    std::vector<StorageBand> bands;
    std::map<Id,Position> recovery;
};
}
