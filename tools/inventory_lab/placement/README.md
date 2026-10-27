# Placement and stash packing proposals

`src/inventory_next/placement.h/.cpp` generates proposals, never changes the live model.

`find_placement` searches caller-ordered destinations, preserving orientation before trying rotation. This supplies the placement part of quick store. The caller supplies actual open-window/rig/category preference order and should omit the current source container when a no-op is unwanted. Each candidate is validated by the same model as manual movement, including growth, acceptance and mass. Search is explicitly bounded; `search_limit` is not proof that no placement exists. Partial stack merging and action/access checks are separate operations.

`plan_stash_sort` requires an explicit stash-policy flag and a world root. Actor grids always reject sorting. It tries category-first and largest-first deterministic layouts, honors blocked cells/compartments, and validates the complete result with atomic relocation. It preserves the existing placement if no candidate is at least as short, and returns an empty successful plan when no change is proposed. Content policy supplies per-item category ranks; numeric category bits are not assumed to be SortingPlus order.

This is a bounded heuristic for up to 256 direct children, not an optimal bin-packing solver. A lower occupied height takes precedence; the category-first candidate wins equal-height ties. Child contents and ownership remain intact. No item is spawned, dropped, merged or deleted by sorting.

Execute nonempty proposals with the model's atomic `relocate`, after rechecking real source access. Moves capture the current session and revision, so intervening changes invalidate the proposal. Permission to sort must come from authored storage role; a world root can also represent floor/corpse data, so world-root kind alone is insufficient authorization.

The background validation compiles debug and release tests for stash-only behavior, ordered fallback, atomic/no-mutation/stale behavior, rotation-only fits, masks and deterministic bounded search. No GUI, installed writes or game launch. Native input integration, real SortingPlus rules and actual stash-role/access discovery remain pending.
