# SQA3 mounted storage and native compile gate

Development only, 27 October 2026. All 77 parity rows remain tracked; no in-game completion claim.

Schema 5 adds stable mounted pouch slots, named compartments, per-instance condition/state/attributes and per-container stack ceilings. Nonempty mounted pouches cannot be removed. Mount failures, damage that would invalidate an occupied slot, and capacity failures preserve the complete model. Real damage/recovery remains an adapter requirement.

Soft rig fullness counts its own occupied grid cells plus occupied granted pouch cells once, excluding pouch shells. Capacity includes currently mounted usable grant cells; authored full exterior must cover that capacity. Blocked gutters are excluded. Recursive validation rejects a pickup if rig growth causes any ancestor overlap/bounds failure. Empty pouches permit folding; this policy is independent of mass. No automatic spill/repack or forced native damage handling is implied.

AccessGate now requires explicit reveal/action permission for each world-owned item ancestor. Presentation retains view-only independent panels, scroll/collapse and equipment selection. EntityRegistry separates core IDs from reused live u16 slots with generation/epoch validation. Placement proposes bounded ordered first-fit/rotation and explicit stash-only repack through the same Model; it never independently changes ownership.

All five core modules are linked in CMake, the native lab library and xrGame's project. Explicit `inventory_next_probe [export]` is registered only with both `-inventory-lab` and `-dbg`. It reads the loaded actor's current native inventory, creates a diagnostic row-packed model and logs mass/ID counts; optional export writes a lab snapshot. IDs/layout are probe-local, not persistent native identity or actual placement. No automatic hook or inventory mutation. Native event/save adapter and interactive view remain open. No game launch authorized or performed.

Local evidence: C:/GAMMA/sqa-previews/inventory-next-20261027/milestone4-growth-build: 54 behavioral checks per debug/release plus independent reload. milestone4-runner-validation/runner-checks-tni19qwo:29 integrations. milestone4-presentation-validation:7 scenario groups per build; milestone4-entity_registry-validation:5; milestone4-placement-validation:5. Real authored geometry contract rerun against current core:2353 cases across56 fixtures, tools/inventory_content/generated/native-contract; synthetic roots, unresolved host item/class/exterior/acceptance remain explicit.

Isolated workflow inventory-next.yml builds DX11-AVX on inventory-next-harness only, read-only repository permissions, artifacts only; no public release or game execution. MSVC result pending at preparation time. GCC results cannot establish native integration.
