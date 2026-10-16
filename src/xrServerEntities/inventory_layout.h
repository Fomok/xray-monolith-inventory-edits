#pragma once

// Persistent outer-inventory coordinates. No object IDs, screen offsets or
// pixels: this value belongs to the item and survives ALife ID remapping.
// This record does not validate occupancy; that is a separate transaction step.
namespace inventory_layout
{
struct Placement
{
    bool valid = false;
    int x = 0;
    int y = 0;
    int width = 0;  // Unrotated footprint, in cells.
    int height = 0;
    bool rotated = false;
    bool manual = false;

    bool set(int nx, int ny, int nw, int nh, bool nr, bool nm)
    {
        // Bound values before narrowing into the packet. Failed writes leave
        // the previous record intact, including its orientation.
        if (nx < 0 || ny < 0 || nx >= 4096 || ny >= 4096 ||
            nw < 1 || nh < 1 || nw > 4096 || nh > 4096)
            return false;
        x = nx; y = ny; width = nw; height = nh;
        rotated = nr; manual = nm; valid = true;
        return true;
    }

    void clear() { *this = Placement{}; }

    template <class Packet> void write(Packet& packet) const
    {
        packet.w_u8(valid ? (1 | (rotated ? 2 : 0) | (manual ? 4 : 0)) : 0);
        if (!valid) return;
        packet.w_u16(static_cast<unsigned short>(x));
        packet.w_u16(static_cast<unsigned short>(y));
        packet.w_u16(static_cast<unsigned short>(width));
        packet.w_u16(static_cast<unsigned short>(height));
    }

    template <class Packet> bool read(Packet& packet)
    {
        const unsigned flags = packet.r_u8();
        if (flags == 0) { clear(); return true; }
        if ((flags & ~7u) || !(flags & 1u)) return false;
        // Separate reads: C++ function argument evaluation order is unspecified.
        const int nx = packet.r_u16();
        const int ny = packet.r_u16();
        const int nw = packet.r_u16();
        const int nh = packet.r_u16();
        return set(nx, ny, nw, nh, (flags & 2u) != 0, (flags & 4u) != 0);
    }
};
}
