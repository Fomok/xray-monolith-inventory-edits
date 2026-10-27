# Squared Away 3.0 / Inventory Next implementation plan — 27 October 2026

Scope: start a fresh native model and test foundation on inventory-next-harness.
Working title: Squared Away 3.0. Fresh code/mod architecture is preferred where it is
more maintainable; no reuse of old SQA code or old-save compatibility is required.
Current public 2.x remains unchanged. The pristine upstream baseline stays at c57fe5b9. Fresh saves are acceptable. This
turn does not integrate the model into xrGame or deliver a GUI/player release.

## Source-informed rules and deliberate choices

Read-only reference: installed KnoxInventoryBeta/42/media/lua/shared/Knox/KnoxGrid.lua
lines 310–458: loose carried weight includes contents; nested bags grow in empty,
up-to-half and greater-than-half steps; small organizers retain size; nestRefusal
checks outer placement and tries relocation. This is behavior research, not a code
port. The completed Mod/UI audit was read before finalizing this slice: C:/GAMMA/sqa-previews/inventory-next-20261027/knox-inventory-audit.md.

Our core will own one unique item tree, independent of displayed panels. Every item
has intrinsic mass excluding child items; recursive sums visit each node exactly
once, including hidden/nested/equipped items. Intrinsic mass includes consumable
contents not represented as distinct child IDs. No weight reduction is applied.
A rigid container has fixed exterior dimensions and accepted category mask. A soft
container has explicit designer-defined empty/half/full sizes, selected by direct
occupied cells (whose child bags already have their recursively computed size).
No copied Knox formulas/configs/assets. Deterministic anchored validation is stricter
than Knox relocation: reject a transaction if any ancestor grows into a neighbor
or outside its grid. Autopacking can be added later as an explicit proposal.

## First slice, in this turn

1. Add renderer-independent C++17 library under src/inventory_next, with its own
   small native test target under tools/inventory_lab/native. No engine headers,
   renderer services, scripting state or parallel replacement UI algorithms.
2. Single-parent item/container/root forest; unique IDs; bounded dimensions/depth;
   cycles/orphans rejected. Root domain distinguishes actor from world. Positions
   and rotation are model data, not inferred from UI visibility.
3. Clone-and-validate atomic move/pickup/drop, revision check for stale requests;
   validate destination restrictions, all capacity and mass bounds, collisions,
   recursive growth through every ancestor. Failure leaves exact state unchanged.
4. Read-only world projections expose already observed world roots/items without
   taking ownership. Discovery, distance/LOS/access and authoritative object checks
   remain engine adapter responsibilities; displaying an item never picks it up.
5. Versioned bounded canonical persistence with integrity check, transactional load,
   durable Windows atomic replacement after flush. Store real model ownership,
   placements, orientation and definitions; not visual contents. No engine-save hook
   until mapping to server entity IDs/configs and save lifecycle is implemented.
6. Compile/run behavioral tests locally using isolated portable compiler if needed:
   nested mass, world previews, pickup/drop, cycles, restrictions, rotation,
   parent/grandparent resize collision rollback, stale revision, persistence restart,
   malformed state/save rejection. Build debug and optimized variants.

## Follow-on engine integration (not in this first slice)

- Adapter maps persistent engine entity IDs to core IDs and item section/config
  definitions. Verify existing Weight() overrides so container contents are not
  counted twice; Inventory.cpp:1038 currently sums all m_all item Weight values.
- Reconcile actual world/actor ownership on load/spawn/destroy. Proposed transfer
  first validates core snapshot, then authoritative engine events, then publishes
  one revision. Any failed engine operation must rollback/reconcile before UI sees
  success. No optimistic model success ahead of event delivery.
- Persistence must be part of engine save transaction, including nested ownership;
  reject missing/duplicate IDs and unknown schema, migrate only explicitly. Core
  standalone save is a test format, not a substitute for engine serialization.
- Native views read same core snapshot: independent scroll/collapse panels; nearby
  containers AND ground items shown before loot. Visibility is access-filtered;
  transfer checks repeated at commit because objects may move/disappear.
- Equipment category can hold multiple compatible items; arrows only change viewed
  selection. Equipped state/mass do not change when cycling. Define slot rules and
  equipment location separate from grid occupancy before implementing.
- Add fixture actor/real entities and -inventory-lab native bootstrap. Coordinator
  owns clean setup at C:/GAMMA/sqa-inventory-lab-game; never launch installed game.
- Gate integration on native drag/equip/transfer/save/reload parity with this library,
  then long-session invalidation, event failure and resource-lifetime tests.

## Explicit gaps

No stacking, shaped grids, autopacking, equipment rules/selection persistence,
UI panels, nearby spatial scan, multiplayer authority, live engine adapter, or
full engine build in this slice. Those cannot be implied by passing core tests.

## Audit reconciliation

Completed Knox audit confirms its reviewed resize gate checks only the immediate
outer container; this core deliberately validates the entire forest/ancestor chain.
Knox small fixed-size exception is capacity-based, not a general rigid-material
policy. SQA3 uses explicit rigid/soft definitions instead. Native weight semantics
are delegated by Knox and not independently proven; hasRoomFor still participates.
Our exact intrinsic-plus-child mass and optional max_contents rules are independent
SQA3 design choices. No claim of unlimited Knox bag weight or copied native behavior.

Later work must add stack-state identity, compartments/masks, transfer lifecycle and
forced-creation recovery. None is hidden behind the current rectangular move tests.
Selection of equipment by stable ID and nearby source handles belongs in the new
contracts, without reusing old SQA wrappers merely for compatibility.

Source adapter detail: CInventory::CalcTotalWeight at Inventory.cpp:1038 sums m_all
Weight(); CInventoryItem::save/load at inventory_item.cpp:390/781 serialize placement
and condition, not this model's full forest. Never substitute this standalone file
for actual engine save integration. Single-player first; multiplayer out of scope.
