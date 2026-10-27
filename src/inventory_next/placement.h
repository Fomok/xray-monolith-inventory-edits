#pragma once
#include "model.h"

namespace inventory_next {
struct PlacementPlan {
    Result result{false,"no_fit"};
    std::vector<Move> moves;
    unsigned probes = 0;
};
// Ordered destinations are supplied by quick-store policy, not map iteration.
// No mutation; execution must recheck access and use the captured revision/session.
PlacementPlan find_placement(const Model&, Id item, const std::vector<Id>& destinations,
                             bool allow_rotation = true, unsigned budget = 8192);
// Explicit authorization by storage policy is required; actor grids always refuse.
// Category ranks are per item, supplied from SortingPlus/content policy.
// Bounded heuristic, never claims optimal packing or proof that no packing exists.
PlacementPlan plan_stash_sort(const Model&, Id stash, bool stash_sort_enabled,
                             const std::map<Id,unsigned>& category_ranks,
                             bool allow_rotation = true);
} // namespace inventory_next
