#pragma once
#include "inventory_grid.h"

namespace inventory_grid
{
// Scratch occupancy for scripted layout planning. One key per visible footprint;
// stacked riders do not occupy another rectangle. This does not transfer objects.
class ScriptGrid
{
public:
    bool resize(int columns, int rows) { return grid_.resize(columns, rows) == Result::ok; }
    bool fits(int x, int y, int width, int height) const
    {
        return grid_.can_place(Placement{0, x, y, width, height, false}) == Result::ok;
    }
    bool occupy(int x, int y, int width, int height)
    {
        if (next_key_ == 0) return false;
        if (grid_.place(Placement{next_key_, x, y, width, height, false}) != Result::ok)
            return false;
        ++next_key_;
        return true;
    }
private:
    Grid grid_;
    Key next_key_ = 1;
};
}
