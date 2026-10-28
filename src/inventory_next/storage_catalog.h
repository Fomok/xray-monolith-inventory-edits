#pragma once
#include "native_layout.h"
#include <cctype>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>

namespace inventory_next {
// SQA authored sizes are rows x columns. Count-only legacy values use the
// default width; the unused tail is blocked rather than granting extra cells.
inline Result parse_storage_shape(const std::string& text,int default_columns,ActorStorage& output) {
    auto trim=[](const std::string& s) {
        auto first=s.find_first_not_of(" \t\r\n");
        return first==std::string::npos?std::string{}:s.substr(first,s.find_last_not_of(" \t\r\n")-first+1);
    };
    const auto value=trim(text);
    auto number=[&](const std::string& raw,int& n) {
        const auto s=trim(raw);
        if(s.empty())return false;
        n=0;
        for(char c:s){if(c<'0'||c>'9'||n>4096)return false;n=n*10+(c-'0');}
        return n>0&&n<=4096;
    };
    int cells=0,columns=default_columns,rows=0;
    auto separator=value.find_first_of("xX");
    if(separator==std::string::npos) {
        if(!number(value,cells)||columns<1||columns>64)return {false,"storage_shape"};
        rows=(cells+columns-1)/columns;
    } else {
        if(!number(value.substr(0,separator),rows)||!number(value.substr(separator+1),columns))return {false,"storage_shape"};
        if(rows>64||columns>64)return {false,"storage_shape"};
        cells=rows*columns;
    }
    if(rows<1||rows>64||columns<1||columns>64||cells>4096)return {false,"storage_shape"};
    ActorStorage candidate;candidate.grid={columns,rows};
    if(cells!=columns*rows){candidate.cells.assign(columns*rows,0);std::fill_n(candidate.cells.begin(),cells,1);}
    output=std::move(candidate);return {true,{}};
}

// Authored rows separated by '|', cells by ','. Zero is blocked; positive
// values identify compartments. A failed parse never modifies the prior shape.
inline Result parse_storage_mask(const std::string& text,ActorStorage& output) {
    if(output.grid.w<1||output.grid.h<1||output.grid.w>64||output.grid.h>64)return {false,"storage_mask"};
    std::vector<unsigned> cells;unsigned rows=0;bool usable=false;
    std::istringstream source(text);std::string row;
    while(std::getline(source,row,'|')) {
        ++rows;std::istringstream columns(row);std::string value;unsigned width=0;
        while(std::getline(columns,value,',')) {
            const auto first=value.find_first_not_of(" \t");const auto last=value.find_last_not_of(" \t");
            if(first==std::string::npos)return {false,"storage_mask"};
            unsigned region=0;
            for(auto i=first;i<=last;++i){const char c=value[i];if(c<'0'||c>'9'||region>4096)return {false,"storage_mask"};region=region*10+unsigned(c-'0');}
            if(region>4096)return {false,"storage_mask"};
            cells.push_back(region);usable=usable||region!=0;++width;
        }
        if(width!=unsigned(output.grid.w)||row.empty()||row.back()==',')return {false,"storage_mask"};
    }
    if(!usable||rows!=unsigned(output.grid.h)||text.empty()||text.back()=='|')return {false,"storage_mask"};
    output.cells=std::move(cells);return {true,{}};
}

struct StorageDefinition {
    ActorStorage interior;
    Size empty,half,full;
    std::string family;
    bool growing=false,allow_container_items=false,independent_exterior=false;
    Mass max_contents=0;
    std::uint32_t stack_limit=0;
    std::vector<unsigned> tear_thresholds;
};
struct StorageCatalog {
    ActorStorage pockets{{4,2},{}};
    ActorStorage default_pack{{7,10},{}};
    std::map<std::string,ActorStorage> sections,kinds;
    std::map<std::string,StorageDefinition> definitions;
    bool apply(const std::string& section,Node& item) const {
        const auto found=definitions.find(section);if(found==definitions.end())return false;
        const auto& definition=found->second;
        item.container=true;item.grid=definition.interior.grid;item.cells=definition.interior.cells;
        item.attributes.erase("storage_torn_closed");
        if(!definition.tear_thresholds.empty()&&!item.cells.empty()) {
            unsigned torn=0;for(auto threshold:definition.tear_thresholds)if(item.condition<=threshold)++torn;
            std::map<unsigned,unsigned> areas;
            for(auto region:item.cells)if(region)++areas[region];
            std::vector<std::pair<unsigned,unsigned>> order;
            for(auto entry:areas)order.push_back({entry.second,entry.first});
            // Released rig rule: largest pockets first; later authored pocket
            // wins ties. Pouch grants are separate nodes and never torn here.
            std::sort(order.begin(),order.end(),std::greater<std::pair<unsigned,unsigned>>());
            for(unsigned i=0;i<torn&&i<order.size();++i)
                for(auto& region:item.cells)if(region==order[i].second)region=0;
            if(std::none_of(item.cells.begin(),item.cells.end(),[](unsigned region){return region!=0;}))
                item.attributes["storage_torn_closed"]="1";
        }
        item.empty=definition.empty;item.half=definition.half;item.full=definition.full;
        item.soft=definition.growing;item.max_contents=definition.max_contents;item.max_stack_quantity=definition.stack_limit;
        item.attributes["storage_family"]=definition.family;
        item.attributes["storage_allow_container_items"]=definition.allow_container_items?"1":"0";
        if(definition.independent_exterior)item.attributes["storage_exterior_independent"]="1";
        return true;
    }
    const ActorStorage& resolve(const std::string& worn_section,const std::string& kind) const {
        if(worn_section.empty())return pockets;
        auto section=sections.find(worn_section);if(section!=sections.end())return section->second;
        auto family=kinds.find(kind);if(family!=kinds.end())return family->second;
        return default_pack;
    }
};
}
