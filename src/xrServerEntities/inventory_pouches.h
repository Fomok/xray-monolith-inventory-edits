#pragma once
#include <array>
#include <string>

namespace inventory_pouches
{
// Fitted pouches are section-valued attachments, not independent contained items.
// The record belongs to the rig and follows it through ALife ID changes.
struct Attachments
{
    bool initialized = false;
    std::array<std::string,2> slots;
    static bool valid_section(const char* section)
    {
        if(!section)return false;
        for(int i=0;section[i];++i)
        {
            const unsigned char c=static_cast<unsigned char>(section[i]);
            if(i>=127 || c<=32 || c>=127)return false;
        }
        return true; // Empty is an explicit removal.
    }
    const char* get(int slot) const { return slot>=1&&slot<=2 ? slots[slot-1].c_str() : ""; }
    bool set(int slot,const char* section)
    {
        if(slot<1||slot>2||!valid_section(section))return false;
        slots[slot-1]=section;initialized=true;return true;
    }
    bool replace(const char* first,const char* second)
    {
        if(!valid_section(first)||!valid_section(second))return false;
        Attachments next;next.set(1,first);next.set(2,second);*this=next;return true;
    }
    void clear() { *this=Attachments{}; }
    template<class Packet> void write(Packet& p) const
    {
        unsigned flags=initialized?1:0;
        for(int i=0;i<2;++i)if(!slots[i].empty())flags|=2u<<i;
        p.w_u8(static_cast<unsigned char>(flags));
        for(const auto& section:slots)if(!section.empty())
        {
            p.w_u8(static_cast<unsigned char>(section.size()));
            for(char c:section)p.w_u8(static_cast<unsigned char>(c));
        }
    }
    template<class Packet> bool read(Packet& p)
    {
        const unsigned flags=p.r_u8();if(flags&~7u || (flags && !(flags&1u)))return false;
        Attachments next;next.initialized=(flags&1u)!=0;
        for(int i=0;i<2;++i)if(flags&(2u<<i))
        {
            const unsigned size=p.r_u8();if(!size||size>127)return false;
            std::string section;for(unsigned j=0;j<size;++j)section+=char(p.r_u8());
            if(section.find('\0')!=std::string::npos || !next.set(i+1,section.c_str()))return false;
        }
        *this=next;return true;
    }
};
}
