#pragma once
#include "model.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <set>

namespace inventory_next {
// Presentation-only bands in the existing native scrolling item list. Grid
// coordinates remain local to their logical owner; headers/recovery are never
// legal drop destinations. No ownership, mass or item state lives in this view.
struct StorageBand {
    Id owner=0; int row=0; Size grid; int column=0; bool collapsed=false; bool accepts_drops=true; bool header=true;
    int group_width=0,group_extra_x=0;float pixel_x=0.0f,pixel_y=0.0f,half_columns=0.0f;
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
        auto add=[&](Id owner,const Size& size,bool header=true,Id group=0,
                     int explicit_row=-1,int explicit_column=0,float pixel_x=0.0f,float pixel_y=0.0f)->Result {
            const bool collapsed=options.collapsed.count(group?group:owner)!=0;
            bool accepts=true;
            for(Id current=owner;current!=1&&current!=2;){
                const auto found=model.nodes().find(current);
                if(found==model.nodes().end()||found->second.location==Location::Overflow||!model.storage_active(current)){accepts=false;break;}
                current=found->second.parent;
            }
            StorageBand band;band.owner=owner;band.row=explicit_row<0?row:explicit_row;band.column=explicit_column;
            band.grid=size;band.group_width=size.w;band.pixel_x=pixel_x;band.pixel_y=pixel_y;
            band.collapsed=collapsed;band.accepts_drops=accepts;band.header=header;
            const auto& n=model.nodes().at(owner);
            for(int y=0;y<size.h;++y)for(int x=0;x<size.w;++x){
                const unsigned region=n.cells.empty()?1:n.cells[static_cast<std::size_t>(y*size.w+x)];
                if(!region)continue;
                const std::pair<int,int> local{x,y},display{x,y};
                if(!band.inverse.emplace(display,local).second)return Result{false,"compartment_projection"};
                band.projected.emplace(local,display);band.regions.emplace(display,region);
            }
            next.bands.push_back(std::move(band));row+=(collapsed?0:next.bands.back().grid.h)+1;
            if(explicit_row>=0)row-=collapsed?1:next.bands.back().grid.h+1;
            next.size.w=std::max(next.size.w,explicit_column+next.bands.back().grid.w);
            return {true,{}};
        };
        auto add_mounts=[&](Id rig)->Result {
            if(options.collapsed.count(rig))return {true,{}};
            const auto first=next.bands.size()-1;const int group_row=next.bands[first].row;
            const int main_width=next.bands[first].grid.w;
            int grant_row=group_row,grant_width=0,grant_index=0;
            std::vector<std::pair<unsigned,Id>> grants;
            for(const auto& pair:model.nodes())if(pair.second.container&&pair.second.location==Location::Mounted&&pair.second.parent==rig)
                grants.emplace_back(pair.second.mounted_slot,pair.first);
            std::sort(grants.begin(),grants.end());
            for(const auto& grant:grants){
                const auto& node=model.nodes().at(grant.second);
                if(const auto result=add(grant.second,node.grid,false,rig,grant_row,main_width,4.0f,float(grant_index));!result)return result;
                grant_width=std::max(grant_width,node.grid.w);
                grant_row+=node.grid.h;++grant_index;
            }
            if(grant_index){
                const int group_width=main_width+grant_width;
                for(std::size_t index=first;index<next.bands.size();++index){
                    next.bands[index].group_width=group_width;
                    next.bands[index].group_extra_x=4;
                    if(index>first)next.bands[index].column+=grant_width-next.bands[index].grid.w;
                }
                next.extra_x=std::max(next.extra_x,4);
                next.extra_y=std::max(next.extra_y,grant_index-1);
                next.size.w=std::max(next.size.w,group_width);
                row=std::max(row,grant_row+1);
            }
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
        if(options.center)for(auto& band:next.bands){
            const int difference=next.size.w-band.group_width;
            band.column+=difference/2;band.half_columns=float(difference%2)*0.5f;
            band.pixel_x+=float(next.extra_x-band.group_extra_x)*0.5f;
        }
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
            return band.half_columns;
        return 0.0f;
    }
    bool pixel_offset(int x,int y,int pitch,float& dx,float& dy) const {
        for(const auto& band:bands)if(!band.collapsed&&band.regions.count({x-band.column,y-band.row})){
            dx=band.pixel_x+band.half_columns*float(pitch);dy=band.pixel_y;return true;
        }
        return false;
    }
    bool hit_pixel(float px,float py,int body,int spacing,int& x,int& y) const {
        if(body<1||spacing<0||!std::isfinite(px)||!std::isfinite(py))return false;
        const int pitch=body+spacing;
        const int approximate_x=static_cast<int>(std::floor(px/float(pitch)));
        const int approximate_y=static_cast<int>(std::floor(py/float(pitch)));
        for(int cy=std::max(0,approximate_y-1);cy<=std::min(size.h-1,approximate_y+1);++cy)
            for(int cx=std::max(0,approximate_x-1);cx<=std::min(size.w-1,approximate_x+1);++cx){
                Id owner=0;Position local;float dx=0.0f,dy=0.0f;
                if(!destination(cx,cy,owner,local)||!pixel_offset(cx,cy,pitch,dx,dy))continue;
                const float left=float(cx*pitch)+dx,top=float(cy*pitch)+dy;
                if(px>=left&&px<left+float(body)&&py>=top&&py<top+float(body)){
                    x=cx;y=cy;return true;
                }
            }
        return false;
    }
    bool centered=false;
    Size size{8,1};
    int extra_x=0,extra_y=0;
    int recovery_row=1;
    std::vector<StorageBand> bands;
    std::map<Id,Position> recovery;
};
}
