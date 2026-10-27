# Squared Away 3.0 background lab — Dev 3

This is a runnable, renderer-independent C++ command lab. It uses the SAME static
inventory core as the native tests. It does not launch Anomaly, create a window,
move the pointer, capture input, or render hidden game frames. No game installation
is needed. This is a development app, not an engine/mod installer.

## Run silently

In the unpacked development package, double-click **inventory-lab-release.exe**.
It is a Windows GUI-subsystem executable with no windows, dialogs or console. With
no arguments it runs the adjacent scenarios/lifecycle.sqa once and exits, writing
a unique runs/run-PID-TICK directory beside the EXE. No Script Host dependency.
The runner sets IDLE priority before work. Processing has a 30-second budget,
max 512 lines, 2048 characters/line and 128 KiB input. No resident loop.

Inside the new run directory:

- result.json: success/failure and final revision/mass, actual priority class and
  whether a console was attached. Opening the EXE alone does not establish success.
- events.jsonl: commands/assertions and expected rejection checks, resulting
  revision/session/high-water ID and carried/projected mass.
- final.inv and named checkpoints: durable schema 4 core state, not Anomaly saves.

The optional hidden PowerShell launcher provides an additional 45-second watchdog
and runs/last-run.json with exact output paths and binary/scenario hashes. The native
EXE default entry does not need PowerShell or Windows Script Host. No services,
scheduled tasks, notifications or permanent background processes are installed.

From an existing terminal or automation (no new visible window):

    powershell.exe -NoProfile -NonInteractive -WindowStyle Hidden -ExecutionPolicy Bypass -File start-background.ps1

Parameters: `-Runner <exe> -Scenario <file> -ResultsRoot <existing-parent> [-Resume <snapshot>]`.
In the source checkout supply `-Runner C:/GAMMA/sqa-previews/inventory-next-20261027/milestone2-build/inventory-lab-release.exe`.
The packaged launcher finds the adjacent EXE automatically. Scenario and executable
SHA256 plus process ID are recorded. Results use a unique child directory; existing
run directories are refused before any previous result is touched.

Direct runner CLI is also windowless:

    inventory-lab-release.exe --scenario scenarios/lifecycle.sqa --output C:/absolute/new-run
    inventory-lab-release.exe --scenario scenarios/resume.sqa --output C:/absolute/new-resume --resume C:/absolute/previous-run/final.inv

Output parent must exist, output directory must not. A seeded scenario cannot also
resume a snapshot. `resume.sqa` checks the final lifecycle snapshot and changes it.
Exit 0 means success; any parse/assertion/persistence failure exits 1. A failure before
output setup may have no result.json; check launch/process exit rather than assuming
success. Snapshot names cannot escape output or address Windows device names.

## Scenario grammar

First line exactly `SQA3_SCENARIO 1` (ASCII/UTF8 without BOM). Whitespace-separated
fields; `#` comments; no executable code or shell commands. Declaration order is
roots/definitions/seeds, then `begin`, then commands. Definitions are named prototype
Nodes used by the core validator at initialize/spawn, not a second inventory model.

    root ID actor|world WIDTH HEIGHT MAX_CONTENT_GRAMS ACCEPTS_MASK
    definition NAME GRAMS CATEGORY EMPTY_W EMPTY_H HALF_W HALF_H FULL_W FULL_H CONTAINER SOFT GRID_W GRID_H ACCEPTS_MASK MAX_CONTENT_GRAMS
    seed ID PARENT DEFINITION X Y ROTATED
    begin

CATEGORY is one bit; ACCEPTS_MASK bitwise acceptance,0allows nothing; grams integer;
rotation/container/soft 0 or 1; max contents 0 means unlimited under our explicit policy.
Root/definition/seed cannot appear after begin. Fresh model can use several roots
for ONE actor's storage domains plus observed world roots; NPC roots are not actor roots.

Commands take exact expected revision and final expected result (`ok` or refusal):

    spawn ID DEFINITION PARENT X Y ROTATED REV EXPECTED
    move ID PARENT X Y ROTATED REV EXPECTED
    mass ID INTRINSIC_GRAMS REV EXPECTED
    erase ID REV EXPECTED
    preview <any command above>
    capture_session
    captured <any command above>
    save NAME
    load NAME
    assert mass GRAMS
    assert count NODE_COUNT
    assert revision REV
    assert high_water ID
    assert world_count COUNT
    assert parent ID PARENT
    assert footprint ID WIDTH HEIGHT

`preview` uses the exact core command on a copy; it neither mutates nor reserves.
The ordinary runner command supplies the CURRENT session token. `capture_session`
and `captured` deliberately submit an earlier token for stale-session tests. Core
callers MUST supply both session and revision; tokens reset after load/initialize.
Tokens are process-local, nonpersisted; external controllers must obtain a fresh
session after reconnect/restart rather than reuse a previous process's token.

Erase is leaf-only: nonempty bags are rejected, never silently spilled/destroyed.
Spawn IDs must exceed the persisted high-water mark; failed proposals do not consume
IDs, committed deleted IDs stay retired. Mass updates revalidate all ancestor/actor
limits. Intrinsic mass excludes items with child IDs, including ammo/fluid only when
not separate child nodes. No quantity/stack or crafting batch transaction is implied.
Schema 4 persists identity, stacks, equipment and cells and rejects earlier schemas; fresh fixtures/saves are intended.

## Tests and remaining work

28 core behavioral tests pass debug/release plus independent-process durable reload.
Runner integration checks cover lifecycle/rigid/resume in both builds, malformed
input, stale sessions/revisions, exact rollback on failed commands, output preservation,
GUI PE subsystem, idle priority/no attached console, and the hidden launcher.
Logs contain expected rejections: pass=true with actual=overlap is a successful test.

Build: build-and-test.ps1. Run integration checks with:

    python validate-runner.py --build <native-build-directory> --output <evidence-parent>

No live renderer/engine validation. CMake target remains supplied but untested here.
World projection is already-observed items only, not actual reachability discovery.
Stacks/partial transfer, equipment locations/groups/arrows, multi-command crafting,
forced-output recovery, native engine entity adapter and save lifecycle remain next.
Engine hook inspection shows InventoryBox::OnEvent immediately mutates parent IDs,
visibility and callbacks; a tested event-completion/reconciliation boundary must
precede integrating this core. No speculative uncompiled engine hook was added.

## Dev3 commands and fixtures

Schema4 now persists fungible stack type/state, per-unit mass, quantity/limit,
equipment definitions/location and authored compartment cells. No heterogeneous
per-member stack object history is claimed. No real item catalog/recipes imported.

Pre-begin definition extensions:

    stack_definition NAME TYPE_ID STATE_KEY QUANTITY LIMIT
    wear_definition NAME ALLOWED_SLOT_BITS REQUIRED_SLOT_COUNT BLOCKED_SLOT_BITS GROUP
    equipment_policy ACTOR_ROOT ALLOWED_SLOT_BITS ACCEPTED_CATEGORIES MAX_MEMBERS
    cells_definition NAME CELL_COUNT ROW_MAJOR_REGION_IDS...
    root_cells ROOT_ID CELL_COUNT ROW_MAJOR_REGION_IDS...

Cell0is blocked, positive values identify separate compartments. An item cannot
bridge regions even when total free area is sufficient. Empty cell data means one
full rectangular region. Slot masks represent explicit gameplay locations/layers;
two-handed items require two chosen slots. Groups only affect read-only browsing.

    split SOURCE QUANTITY NEW_ID DESTINATION X Y ROTATED REV EXPECTED
    merge SOURCE TARGET QUANTITY REV EXPECTED
    equip ITEM ACTOR_ROOT SLOT_BITS REV EXPECTED
    snapshot NAME
    assert quantity ID QUANTITY
    assert equipped ID SLOT_BITS
    assert group_count GROUP COUNT
    assert cycle GROUP SELECTED_ID DIRECTION EXPECTED_ID

Move on equipped items means validated unequip/drop into a real grid. Equipped bags
still have full ownership/mass and changing exterior, but occupy no actor grid cells.
`mass` sets per-unit intrinsic grams on stack records. Merge requires matching type,
state, category, per-unit mass, limit and footprint. Only explicitly fungible,
noncontainer/nonwearable nodes may stack. Exhausted source IDs remain retired.

Compound commands:

    batch_begin
    erase INPUT_ID REV queued
    spawn OUTPUT_ID DEFINITION PARENT X Y ROTATED REV queued
    batch_end ok

Ordered batches validate each stage on a copy, then publish once; if output creation
fails, consumed inputs stay intact. All staged requests carry the same base revision.
For occupied swaps/permutations use the distinct final-state transaction:

    layout_begin
    move FIRST PARENT X Y ROTATED REV queued
    move SECOND PARENT X Y ROTATED REV queued
    layout_end ok

Only moves in a layout; duplicate item targets rejected. No temporary fake cells.
Each compound operation accepts at most64commands. No guarded compound engine action
is implied: world authorization must be rechecked by the eventual native adapter.

Access-boundary scenarios:

    observe WORLD_ROOT EXISTS REACHABLE
    capture_access
    guarded <ordinary command>
    preview guarded <ordinary command>
    guarded_captured <ordinary command>
    assert accessible_count CONTAINER COUNT

Observations are synthetic inputs in this test lab. AccessGate rechecks source and
destination ancestry; loss of reachability invalidates tickets even without model
mutation. Load clears observations. This is not actual world discovery/LOS/security.

New scenarios: stacks-equipment.sqa, crafting.sqa, swap.sqa, compartments.sqa.
`snapshot NAME` writes snapshot-NAME.json through the shared read API. Offscreen
inventory-snapshot.exe consumes .inv saves and writes diagnostic BMP files without
opening windows; see source/tools/inventory_lab/viewer/README.md. It is not an
interactive interface or game screenshot.46 core checks per build,27 runner checks,
10 renderer checks passed; full77-row parity remains tracked in PARITY-COVERAGE.md.
