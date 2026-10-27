#pragma once
#include "native_layout.h"
#include <cctype>
#include <map>
#include <string>

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

struct StorageCatalog {
    ActorStorage pockets{{4,2},{}};
    ActorStorage default_pack{{7,10},{}};
    std::map<std::string,ActorStorage> sections,kinds;
    const ActorStorage& resolve(const std::string& worn_section,const std::string& kind) const {
        if(worn_section.empty())return pockets;
        auto section=sections.find(worn_section);if(section!=sections.end())return section->second;
        auto family=kinds.find(kind);if(family!=kinds.end())return family->second;
        return default_pack;
    }
};
}
