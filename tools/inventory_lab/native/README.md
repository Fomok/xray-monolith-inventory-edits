# Squared Away 3.0 / Inventory Next native core

Current milestone: [Background Lab Dev 2](BACKGROUND-LAB.md) adds atomic spawn,
leaf deletion, intrinsic mass changes, command preview and session tokens, schema2
identity persistence, named fixtures and a hidden command runner.28 core tests pass
per build, plus independent reload;19 runner integration checks pass. The first-slice
record below is historical where superseded by BACKGROUND-LAB.md.

This is a new C++17 library, not the current SQA grid and not integrated into xrGame.
Working title is Squared Away 3.0, with fresh code and fresh saves; old SQA code/save
compatibility is not required. Existing public 2.x remains unchanged.
It is built from src/inventory_next/model.cpp by the native driver and is intended
to be the SAME linked model used by the later engine adapter. No renderer, engine,
Lua or game assets are needed for these tests. Windows persistence is a separate unit.

## What is implemented

- A forest with unique IDs, one parent per item, ActorRoot/WorldRoot owners and
  real nested container records. No visible-panel membership is used for ownership.
- Integer intrinsic grams plus recursive child mass, counted once per ID. A bag
  contributes its own mass and all contents. ActorRoot totals exclude WorldRoot
  trees. Previewing already observed floor items/containers does not pick them up.
- Rectangular grid positions and rotation; rigid exterior stays fixed. Optional
  category-bit restrictions and recursive contents-mass limits apply to containers
  and actor roots. Zero mass limit is explicitly unlimited in THIS model; this is
  a prototype configuration convention, not a claim about Knox transfer behavior.
- Soft exterior sizes are explicit empty/half/full dimensions in definitions.
  Direct occupied cell area includes current child bag sizes. More than half the
  internal cells selects full exterior; bottom-up calculation propagates through
  every ancestor. Full exterior area cannot be below internal grid area.
- Move, pickup, drop and reposition all use one clone/validate/publish operation.
  A stale revision, bad parent, restriction, cycle, overlap, out-of-bounds growth
  or recursive mass overflow fails without changing any ownership/placement/revision.
  Anchors and rotations stay fixed; no implicit packing or relocation.
- Schema-v1 canonical text with CRC32; bounded parser, strict identity/graph checks,
  and atomic in-memory load. Stores ownership, masses, definitions, position,
  orientation and revision, rather than reconstructing from visible box contents.
- Windows durable-file helper writes a separate same-directory temporary file,
  flushes it, closes it, then MoveFileExW replaces the target with write-through.
  No remove-then-rename window. Failed replacement retains the old save and removes
  this helper's temp file. Crash-stranded temps are ignored, not auto-loaded.

## Build and tests

From PowerShell (paths can be changed):

    ./tools/inventory_lab/native/build-and-test.ps1 `
      -Compiler C:/GAMMA/sqa-previews/inventory-next-20261027/toolchain/w64devkit/bin/g++.exe `
      -OutputDirectory C:/GAMMA/sqa-previews/inventory-next-20261027/native-build

The script compiles a static library and links the driver to it separately in debug
(-O0 -g -D_GLIBCXX_ASSERTIONS) and optimized (-O2 -DNDEBUG), with warnings as errors.
Assertions in the driver remain enabled in both variants. Each executes 21 grouped
behavioral tests, including 600 repeated pickup/drop transactions, then starts an
independent process which reloads the durable file and checks ownership/revision/mass.
CMake targets are provided, but this machine validated the direct GCC script, not CMake.

Portable official w64devkit 2.10.1 (GCC 16.2.0) was extracted under the evidence folder;
no system installation. Download archive SHA256:
add1edf984ce4d473cfeb7c52e0ab38db73a855bba6c7f564d5e16648eb2896a
Source: https://github.com/skeeto/w64devkit/releases/tag/v2.10.1
This toolchain can build this independent core; it is not a VS2022 engine toolchain.

## Contracts and limits

Model is single-threaded and scoped to one actor's observed ownership forest (multiple
ActorRoot storage domains can contribute to that one actor). Callers cannot mutate
nodes through getters. initialize is bootstrap/reset, not an ordinary gameplay edit.
After initialize/load, the adapter must discard old UI requests/session tokens: a
saved revision is not a globally unique epoch. move revisions protect requests only
within that initialized session. No node spawn/delete or intrinsic-mass update API
exists yet; adapters must not bypass atomic validation to implement those operations.

Intrinsic mass excludes independently represented child IDs; contents such as water
or rounds are included only if not themselves separate child nodes. The future
engine adapter must map and quantize Weight() correctly once, and remove existing
double aggregation. This implementation does not change current engine weight.

World projection returns direct children of observed WorldRoot domains. It is a
read-only inventory view, NOT a spatial query, LOS/access check, ownership authority
or permission to transfer arbitrary world entities. Nested contents are not exposed
by that floor projection. The adapter must discover/filter objects and recheck
availability/distance/rights at commit, then synchronize engine event results.

Limits: 4096 nodes, 64 levels, grid/exterior dimensions at most 64x64, forest total
mass at most 10^12 grams, save at most 2MiB. Full snapshot validation and pairwise
rectangle tests prioritize correctness; no runtime performance target is claimed.
No stacking, shape masks, equipment/category arrows, UI scrolling/collapse, model
spawn/delete/update, game save hooks, multiplayer or renderer integration yet.

CRC32 detects corruption, not adversarial authenticity. Definitions stored in this
lab save are test inputs; an engine adapter must resolve authoritative section
configs and entity IDs rather than trust saved definitions as config authority.
File helper assumes one writer; it has no concurrent save ordering/backup journal.
Tests cover flush/replacement APIs and restart, not physical power-loss simulation.
The game must eventually include model state in its own save transaction; this
standalone format is not an Anomaly save or migration promise.

## Development setup

Implementation plan: docs/inventory-next/IMPLEMENTATION-PLAN.md.
Coordinator prepared C:/GAMMA/sqa-inventory-lab-game from the local full Anomaly1.5.3
archive. Evidence C:/GAMMA/sqa-previews/inventory-next-20261027/clean-lab-install.json:
95files,17,359,866,666bytes, extraction CRC passed, own root-local appdata, no saves
copied, no game launch. Stock archive engines only at handoff; pinned upstream
runtime/data overlay and native adapter are not installed. User does not need to
download another base copy. Do not launch this as an Inventory Next test yet.

No installed game/mod changes, public release, website or production-engine update.
Reference reads: KnoxInventoryBeta shared KnoxGrid.lua/KnoxFootprint.lua and client
KnoxContainerWindow.lua, plus Anomaly filesystem config. No Knox code/assets copied.
