# Normalized rules v1: engine handoff

Run `validate_catalog.py` as documented in README. It now runs both test suites,
rebuilds the catalog and rules twice, and emits `rules-v1.json`,
`rule-fixtures-v1.json`, `validation.json` and the test transcript in the requested
content-validation directory. To compile a previously extracted catalog:

```powershell
python -B tools/inventory_content/compile_rules.py --catalog C:/GAMMA/sqa-previews/inventory-next-20261027/content-validation/catalog-v1.json --output C:/GAMMA/sqa-previews/inventory-next-20261027/content-validation/rules-v1.json
```

Rules are independent JSON, schema `sqa.inventory-rules` version `1.0.0`, linked to
the exact catalog SHA-256. Runtime-derived semantics are hash-gated against the
audited full-release scripts. A changed runtime hash fails compilation until its
semantics are re-audited. Config field provenance and runtime line ranges are
included. This is newly written compiler/evaluator code, not copied Lua wrappers.

## Direct mappings to current model.h

`geometry.grid` maps to `Node.grid`; `geometry.cells` is row-major, zero blocked,
positive per-compartment region IDs; `compartment_names` maps to the native map.
Every authored rectangle receives its own region, even when edges touch. A native
placement must stay inside one positive region. Rotation remains native behavior.
Base rig names `b1..bN` follow canonical 2x2, 3x1, 2x1, 1x1 order; coordinates follow
the same-kind occurrence in the authored layout. The band is a bound, not storage
width. Actual extents determine width/height, preserving blocked gaps.

Box slots use across/down/width/height, one-based in source and zero-based in output.
`amp_box_cells` creates one region per cell; ordinary boxes have one full-grid region.
Invalid/overlapping authored masks remain unresolved instead of applying legacy
slot-dropping/fallback behavior that could silently widen acceptance. Missing rig
layouts are unresolved, not guessed packed coordinates. Full-release Nylon and its
alias are examples. Backpack geometry uses authored rows-first sizes and preserves
cell count; this is not a UI-width reflow algorithm or an exterior footprint.

Mount IDs 1 and 2 are local to the owning rig, named `p1` and `p2`; they are not
native equipment slot IDs or globally allocated entity IDs. Conditions map directly
to `minimum_condition` basis points 1000 and 5000. `accepts_predicate` is symbolic;
do not cast it to a category bit. Pouch grant templates keep names `p1_1`, `p1_2`,
`p2_1`, etc. Removing mount 1 never renumbers mount 2. Each grant has a complete
local rectangular grid; its placement in the parent rig is deliberately null.
The three external magazine-pouch grants come from the explicitly audited runtime
table, which takes precedence over section keys; their item definitions remain
external. Tall-pouch grants come from authored `amp_pgrant_*` fields.

## Exact extensions or adapter decisions still required

1. `MountSlot.footprint` and `accepts` require resolved host item geometry/category
   mapping. A 1x1 placeholder is not supplied. Mount acceptance is based on a
   nonempty pouch grant, not arbitrary native equipment slot assumptions.
2. Model cells describe one static parent grid. Dynamic pouch grants need either
   independently owned child compartment grids or an atomic geometry-update command
   with stable region/name mapping. Appending and renumbering parent regions on
   each fit/removal is unsafe. Grant coordinates have no fixed authored parent-grid
   location in source. Do not infer them from presentation packing.
3. Condition-disabled mounts retain fitted pouches. The source separates slot
   availability from ownership. Native retention/hidden-grant behavior and recovery
   must be decided explicitly; do not delete contents or detach automatically.
4. Fan document storage ignores each item's normal footprint. Current rectangular
   cells cannot express a per-destination footprint policy. `ampbox_docs` remains
   unresolved pending a dedicated ordered-list domain or destination shape policy.
5. `Node.accepts` bit masks alone cannot express exact section `never`/`also`,
   class-prefix exceptions and host ammo classification. Add an adapter predicate
   gate shared by preview and commit, or an equivalent native content-policy ID.
   Missing family `anything` is unresolved for SQA3: legacy fallback allows
   non-box/non-rig items, but that must not silently dictate the new nesting policy.
6. Rigid/soft classification, growth and native nesting policy are still separate.
   Legacy blanket box-in-box rejection is recorded as historical behavior, not
   compiled as an SQA3 rule. Family rig gates and explicit exclusions remain data.
7. `Node.max_stack_quantity` can consume authored box ceilings. Inventory ceilings
   need the compiled ordered policy plus live settings. Stackability/state equality
   remains a separate native admission rule; a ceiling does not prove stackability.

## Policy semantics and evaluators

Inventory stack precedence: menu section, menu kind, exact file section, literal
substring (longest then lexical), boolean flag (lexical), kind, then one. File
matches use `max(1,floor(base*multiplier+0.5))`; menu overrides are absolute. Disabled
stacking gives one. Unknown host kind/flags must be resolved before using the
reference evaluator; they are not inferred from item names.

Acceptance families compile lists/flags and order: rig gate, never, also, allowed
rigs, host ammo predicate, is_mag, weapon class, kind, deny. Weapons have `WP_`
prefix excluding `WP_SCOPE`, `WP_SILEN`, `WP_GLAUN`. `accepts()` returns null when
required host facts are absent. It evaluates family rules only, not nesting or
geometry. The engine must combine these checks atomically.

Fixtures cover bandolier, carrier, food box, hip pouch, pistol case, artifact bag,
all five pouch grants and authored backpack sizes. Tests check known cell arrays,
distinct touching regions, blocked holes, overlap/out-of-bounds refusal, canonical
IDs, retained second-mount names, stack precedence/scaling, acceptance exclusions,
runtime hash gating and deterministic output. No native/gameplay validation is
claimed. Core files and vault remain owned by the engine/coordinator chats.

## Actual shared-model integration contract — 27 October 2026

`run-native-contract.ps1` launches the generator hidden at Idle priority. The
generator snapshots current `model.cpp`/`model.h`, generates C++ fixtures directly
from compiled rules, and compiles the unchanged shared implementation using the
requested portable GCC. Compilation is bounded to 40 seconds; execution to 10.
Both child processes use CREATE_NO_WINDOW and IDLE_PRIORITY_CLASS. The contract
executable asserts no attached console and Idle priority before running cases.

Run from the repository root:

```powershell
./tools/inventory_content/run-native-contract.ps1
```

The completed run passed **2,353 native cases across 56 geometry fixtures**,
including real rig/box interiors, backpack capacities and isolated pouch-grant
grids. It checked every usable/blocked cell, every neighboring distinct-region
boundary, each full compartment and its rotated equivalent, simultaneous filling
of every usable cell, atomic rejection when full, preview/commit agreement and
region/capacity persistence. Compilation produced no warnings. Core hashes were
unchanged between snapshot and run completion:

- model.cpp: `a5fbe3268c2786c2c5addbe1494ada94063f7bcfe8c46e278618294c446797ff`
- model.h: `1c6948960044306d20bb7823844e606866e8255d19bf175740c8c5969c3f6d6e`
- rules: `dcebc2a4276f62075d341aec8cdb37f0c8c62a75b30c7bd5638b51318ec07f70`

Generated evidence stays under `tools/inventory_content/generated/native-contract/`:
`evidence.json` pins source/compiler/test hashes, commands, exit codes and timing;
`coverage.json` maps fixtures to authored source fields and explicit skipped scopes;
`fixtures.inc` is the generated C++ binding; `step-1.stdout.txt` lists each passed
contract. The copied core snapshot and compiled executable are evidence only and
are ignored by Git. No core files, package or commit were modified.

**Boundary:** real interior geometry is bound to synthetic actor-root test storage;
probe items and the staging world grid are explicit test infrastructure. These are
not fabricated full host items. Full-item exterior/class/acceptance and pouch-mount
transactions remain skipped. Unsupported Nylon layouts and fan storage are skipped,
not replaced by rectangles. The coverage includes 44 explicit skip records plus
the unresolved source-rule list. No gameplay or full content admission is claimed.

**Next missing behavior:** bind the compiled pouch grants to real mounted pouch
nodes with resolved exterior/category data and test mount/unmount/condition changes
without moving the other mount's contents. The current core already lets mounted
pouches own their grids; the remaining content adapter must assemble multi-grant
storage into that representation with stable names. The current static geometry
contract does not execute Mount or settle presentation packing. Acceptance families
also still require a predicate adapter shared by preview and commit.
