#pragma once
// Geometry/occupancy only. Callers supply one key per visible footprint (item
// or stack). Object ownership, stack eligibility and persistence belong to the
// engine integration layer; UI coordinates must never be used as authority.
#include <algorithm>
#include <cstdint>
#include <vector>

namespace inventory_grid
{
using Key = std::uint32_t;

enum class Result
{
    ok,
    invalid_size,
    outside,
    overlap,
    missing_item,
    duplicate_key
};

struct Placement
{
    Key key;
    int x;
    int y;
    int width;  // Unrotated footprint, in cells.
    int height;
    bool rotated;

    int columns() const { return rotated ? height : width; }
    int rows() const { return rotated ? width : height; }
};

class Grid
{
public:
    // A zero-sized grid is disabled until a valid resize.
    Grid() : columns_(0), rows_(0) {}

    int columns() const { return columns_; }
    int rows() const { return rows_; }
    const std::vector<Placement>& entries() const { return entries_; }

    const Placement* find(Key key) const
    {
        const auto it = std::find_if(entries_.begin(), entries_.end(),
            [key](const Placement& p) { return p.key == key; });
        return it == entries_.end() ? nullptr : &*it;
    }

    Result resize(int columns, int rows)
    {
        if (!valid_size(columns, rows))
            return Result::invalid_size;
        for (const Placement& p : entries_)
            if (!inside(p, columns, rows))
                return Result::outside;
        columns_ = columns;
        rows_ = rows;
        return Result::ok;
    }

    Result can_place(const Placement& p) const
    {
        return check(p, p.key, false, 0);
    }

    // Existing placement is unchanged on every validation failure.
    Result place(const Placement& p)
    {
        const Result result = can_place(p);
        if (result != Result::ok)
            return result;
        for (Placement& current : entries_)
            if (current.key == p.key)
            {
                current = p;
                return Result::ok;
            }
        entries_.push_back(p);
        return Result::ok;
    }

    bool erase(Key key)
    {
        const auto it = std::find_if(entries_.begin(), entries_.end(),
            [key](const Placement& p) { return p.key == key; });
        if (it == entries_.end())
            return false;
        entries_.erase(it);
        return true;
    }

    Result move(Key key, int x, int y, bool rotated)
    {
        const Placement* current = find(key);
        if (!current)
            return Result::missing_item;
        Placement next = *current;
        next.x = x;
        next.y = y;
        next.rotated = rotated;
        return place(next);
    }

    // Query only: does not reserve or move anything. Deterministic row-major
    // search; try the current orientation before optional autorotation.
    Result first_fit(Key key, int width, int height, bool rotated,
        bool allow_rotation, Placement& out) const
    {
        if (!valid_size(width, height))
            return Result::invalid_size;
        for (int pass = 0; pass < (allow_rotation && width != height ? 2 : 1); ++pass)
        {
            Placement p{key, 0, 0, width, height, pass == 0 ? rotated : !rotated};
            const int last_x = columns_ - p.columns();
            const int last_y = rows_ - p.rows();
            for (int y = 0; y <= last_y; ++y)
                for (int x = 0; x <= last_x; ++x)
                {
                    p.x = x;
                    p.y = y;
                    if (can_place(p) == Result::ok)
                    {
                        out = p;
                        return Result::ok;
                    }
                }
        }
        return Result::outside; // No valid destination; out is unchanged.
    }

    Result transfer(Key key, Grid& target, int x, int y, bool rotated)
    {
        if (&target == this)
            return move(key, x, y, rotated);
        const Placement* current = find(key);
        if (!current)
            return Result::missing_item;
        if (target.find(key))
            return Result::duplicate_key;
        Placement next = *current;
        next.x = x;
        next.y = y;
        next.rotated = rotated;
        const Result result = target.can_place(next);
        if (result != Result::ok)
            return result;
        // Allocate before removing the source: even an allocation failure
        // cannot leave the source item missing from both grids.
        target.entries_.push_back(next);
        erase(key);
        return Result::ok;
    }

    // Replace state only if the entire restored snapshot is valid.
    Result restore(int columns, int rows, const std::vector<Placement>& entries)
    {
        Grid candidate;
        Result result = candidate.resize(columns, rows);
        if (result != Result::ok)
            return result;
        for (const Placement& p : entries)
        {
            if (candidate.find(p.key))
                return Result::duplicate_key;
            result = candidate.place(p);
            if (result != Result::ok)
                return result;
        }
        entries_.swap(candidate.entries_);
        columns_ = columns;
        rows_ = rows;
        return Result::ok;
    }

    Result swap(Key key, Grid& other, Key other_key)
    {
        const Placement* a = find(key);
        const Placement* b = other.find(other_key);
        if (!a || !b)
            return Result::missing_item;
        if (&other == this && key == other_key)
            return Result::ok;
        if (&other != this && (find(other_key) || other.find(key)))
            return Result::duplicate_key;
        Placement next_a = *a;
        Placement next_b = *b;
        next_a.x = b->x;
        next_a.y = b->y;
        next_b.x = a->x;
        next_b.y = a->y;
        Result result;
        if (&other == this)
        {
            result = check(next_a, key, true, other_key);
            if (result != Result::ok) return result;
            result = check(next_b, key, true, other_key);
            if (result != Result::ok) return result;
            if (intersects(next_a, next_b)) return Result::overlap;
        }
        else
        {
            result = other.check(next_a, other_key, false, 0);
            if (result != Result::ok) return result;
            result = check(next_b, key, false, 0);
            if (result != Result::ok) return result;
        }
        // Both pointers still refer to their original entries; no allocation
        // occurred during validation. Save indices before changing either key.
        const auto index_a = static_cast<std::size_t>(a - entries_.data());
        const auto index_b = static_cast<std::size_t>(b - other.entries_.data());
        entries_[index_a] = next_b;
        other.entries_[index_b] = next_a;
        return Result::ok;
    }

private:
    int columns_;
    int rows_;
    std::vector<Placement> entries_;

    static bool valid_size(int columns, int rows)
    {
        return columns > 0 && rows > 0 && columns <= 65535 && rows <= 65535;
    }

    static bool inside(const Placement& p, int columns, int rows)
    {
        return valid_size(p.width, p.height) && p.x >= 0 && p.y >= 0 &&
            p.columns() <= columns && p.rows() <= rows &&
            p.x <= columns - p.columns() && p.y <= rows - p.rows();
    }

    static bool intersects(const Placement& a, const Placement& b)
    {
        return a.x < b.x + b.columns() && b.x < a.x + a.columns() &&
            a.y < b.y + b.rows() && b.y < a.y + a.rows();
    }

    Result check(const Placement& p, Key ignore, bool ignore_second, Key second) const
    {
        if (!valid_size(p.width, p.height))
            return Result::invalid_size;
        if (!inside(p, columns_, rows_))
            return Result::outside;
        for (const Placement& current : entries_)
            if (current.key != ignore && !(ignore_second && current.key == second) &&
                intersects(p, current))
                return Result::overlap;
        return Result::ok;
    }
};
} // namespace inventory_grid



