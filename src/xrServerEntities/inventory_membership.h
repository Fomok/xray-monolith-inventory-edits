#pragma once
#include "inventory_layout.h"
#include <cstdint>
#include <limits>
#include <string>

namespace inventory_membership
{
using Identity = std::uint64_t;
// Allocated lazily after objects are loaded. Both rig identities and references
// advance the watermark on read, regardless of serialization order. No ALife IDs
// or Lua numbers are used as persistent identities.
inline Identity& watermark() { static Identity value = 0; return value; }
inline void observe(Identity value) { if (value > watermark()) watermark() = value; }
inline Identity allocate()
{
    if (watermark() == (std::numeric_limits<Identity>::max)()) return 0;
    return ++watermark();
}
template<class Packet> void write_identity(Packet& p, Identity value)
{ for (int i=0;i<4;++i) p.w_u16(static_cast<unsigned short>(value >> (i*16))); }
template<class Packet> Identity read_identity(Packet& p)
{ Identity value=0; for(int i=0;i<4;++i) value |= Identity(p.r_u16()) << (i*16); return value; }
struct Membership
{
    Identity owner = 0;
    std::string pocket;
    inventory_layout::Placement layout;
    int order = 0;
    bool valid() const { return owner != 0 && layout.valid && !pocket.empty(); }
    const char* key() const { return pocket.c_str(); }
    bool set(Identity rig, const char* key, int x, int y, int w, int h, bool rotated, int rank)
    {
        if (!rig || !key || rank<0 || rank>=65536) return false;
        std::string name;
        for (int i=0;key[i];++i)
        {
            const char c=key[i];
            if(i>=63 || !((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_')) return false;
            name += c;
        }
        inventory_layout::Placement place;
        if(name.empty() || !place.set(x,y,w,h,rotated,true)) return false;
        owner=rig;pocket=name;layout=place;order=rank;return true;
    }
    void clear() { *this=Membership{}; }
    template<class Packet> void write(Packet& p) const
    {
        p.w_u8(valid()?1:0);if(!valid())return;
        write_identity(p,owner);p.w_u8(static_cast<unsigned char>(pocket.size()));
        for(char c:pocket)p.w_u8(static_cast<unsigned char>(c));
        layout.write(p);p.w_u16(static_cast<unsigned short>(order));
    }
    template<class Packet> bool read(Packet& p)
    {
        const unsigned flag=p.r_u8();if(!flag){clear();return true;}if(flag!=1)return false;
        const auto rig=read_identity(p);const unsigned n=p.r_u8();if(!n||n>63)return false;
        std::string key;for(unsigned i=0;i<n;++i)key+=char(p.r_u8());
        inventory_layout::Placement place;if(!place.read(p)||!place.valid)return false;
        const int rank=p.r_u16();
        if(key.find('\0')!=std::string::npos || !set(rig,key.c_str(),place.x,place.y,place.width,place.height,place.rotated,rank))return false;
        observe(rig);return true;
    }
};
}
