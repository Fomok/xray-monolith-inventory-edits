# Milestone 3 — stack identity, equipment locations, UI read model

Scope: substantial continuation of SQA3 parity, not a declaration of full parity.
Implement fungible stack records with stable stack IDs, explicit type/state keys,
per-unit intrinsic grams, quantity/limit, atomic split/merge and exact conservation.
Containers/equippable stateful items remain quantity1; no automatic merges by category.
Split keeps source ID and allocates a new high-water ID; merge keeps target ID and
retires exhausted source ID. Matching includes state, mass, footprint and limit.

Equipment is an explicit location outside grid occupancy. Ownership still points to
actor root, so recursive mass/contents stay authoritative. Root defines allowed slot
bits, accepted categories and max equipped count. Item declares supported slots and
required slot count; chosen occupied bits are recorded on equip. Overlap/block masks
reject incompatible layering. Multiple compatible slots can share a presentation
category. Group cycling is a read-only ID selector, not a mutation/equip action.
Equipped soft bags still grow but do not collide with actor grid; unequip is validated
against current full footprint. No old SQA code/save compatibility obligation.

Expose immutable item/group snapshots with identity, quantity, ownership, location,
footprint, gross mass and equipment selection. Runner syntax/JSON snapshot extends
same native library. Schema4 persists stack/equipment rules/state and authored compartment cells. Audit parity
file once available and preserve explicit incomplete features.

Next layer: adapter with observed source availability/reachability revision separate
from model revision, transactional gate before core commit. Keep it testable against
headless observations and mark actual engine discovery/events not connected. No fake
claim of native engine or rendered UI validation. Run bounded hidden local tests and
publish dev source only; coordinate UI source ownership through existing coordinator.

## Actual implementation and tests

46 behavioral tests pass per debug/optimized build, plus separate-process persistence.
27 bounded runner integration checks include stack/equipment/access, crafting batches,
occupied-cell swaps and compartment fixtures.10 hidden GDI renderer checks passed on
schema4; equipment and rotated compartment images were inspected. Model source remains
the only packing/mass authority. Public/pristine unchanged, no game launch or input.

Added explicit execute_batch (ordered legal stages, one publish/revision) and relocate
(simultaneous owner/position staging, one final validation, duplicate targets refused).
Do not use ordered batches as occupied-cell swap emulation. Both rollback exact state.
Single-command AccessGate observes availability/reach independently of inventory revision;
all observations invalidate on model reload. This is an injectable domain boundary, not
live world discovery or engine event delivery. Unguarded batches are not a world-action API.

Node.cells is row-major:0blocked, positive compartment IDs, empty means one full rectangle.
A rectangle must stay inside one nonzero region. Soft growth uses usable cell count.
Total authored cells capped65536; individual grid64x64. Model::cell_region supplies
viewer cell shading; renderer never independently judges fit. Additional node fields
for stacks/equipment and regions are stored in schema4; earlier lab schemas rejected.

Full accepted inventory scope is tracked in PARITY-COVERAGE.md:77 audited rows,32 with
explicitly partial core evidence and no row certified complete in game. Current request
is NOT finished. Next integration stages and remaining content/UI policies are listed
there; existing SQA/Knox behavior cannot be inferred from generic tested primitives.
