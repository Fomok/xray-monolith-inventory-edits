# Public Squared Away inventory engine handoff — 29 October 2026

This branch is `inventory-public-layouts-nesting` in `C:/GAMMA/sqa-engine-public-layouts-nesting`. It began at `fc3e34ff671de183dfef167294e89048a2a118a9`, the target of the latest public **standard** engine release `engine-2026.10.11`. `origin/amp-hooks-mt` contains that commit. The public all-DX asset has SHA256 `c9c657003022b48e13168e33fad1e924a61ab2c1104a5a8fe7c0ffb3f4e16504`. No Bodycam, DLSS or FG changes are in this branch. The public mod source target is full 2.1.0 Hotfix 1, not the paused SQA3 rewrite.

## Native API contract for Mod/UI

- `actor:sqa_storage_transfer(item, destination)` dispatches direct SELL/BUY without an actor-pocket staging step. `destination` can be the actor, a portable container, or a stash. The item can be a leaf or a loaded portable container. The source can be actor, portable container, or stash. Source membership, live IDs, source stash `can_take`, destination lifetime, and ancestry cycles are checked before dispatch. The UI must still preflight the exact target cell, size, capacity, access/range and item category, then recheck its own layout record after the transfer settles.
- `actor:sqa_equip_from_container(item, source, slot, return_to_source)` accepts a loaded portable container from a stash or portable container as well as an ordinary item. It reserves the target native slot and can return the displaced item directly to the source. If the source is inside the displaced container, the cycle is refused. UI preflight must check the displaced item's fit in its exact source before calling.
- `actor:sqa_rig_transfer_count()`, `sqa_rig_transfer_pending(id)` and `sqa_rig_transfer_rig_pending(id)` remain the async ownership barriers. Do not serialize a target placement as complete until parent/ownership arrival is observed.
- `container:iterate_container(callback, context)` still visits direct children only. The UI can recurse into a child that `is_container()` returns true for, checking actual `parent()` at each level. Native actor possession, quest enumeration and mass now walk nested carried children.
- The engine does not choose a pocket or grid position. Keep the public mod's existing saved placement metadata; do not import SQA3 ledger or save format. Exact destination is the UI's responsibility.

## Verification boundary

The changes are source-only until GitHub Actions finishes the new focused DX11 AVX workflow. No public release, production install, MO2 edit or gameplay test is claimed. The current Dev32 test process was absent when checked; the paused SQA3 lab remains separate. Final production installation is coordinated after matching engine/mod packaging and backups.
