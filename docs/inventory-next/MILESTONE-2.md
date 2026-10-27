# Milestone 2 — headless command lab

Scope selected 27 October 2026: close atomic item lifecycle gaps and deliver a real
file-driven background executable using the same static C++ core. No xrGame launch,
GUI, offscreen capture, focus/input control, CI or public/pristine branch changes.

Implement spawn, leaf-only erase, intrinsic mass changes and move through one atomic
validator. Reject nonempty deletion, roots, stale revisions and invalid definitions.
Maintain a persisted ID high-water mark: failed spawn does not consume it; committed
IDs are never reused even after deletion/load. Schema2 explicitly replaces lab v1.
Add read-only preview using that exact command path on a copy; commit revalidates the
revision and full model, not an untrusted preview result. Preview does not reserve.

Runner: named definitions, initial fixture declarations, command/preview expectations,
assertions and save/reload steps in bounded text scenarios. Fixed grammar, no shell,
script eval or game calls. Emit command JSONL plus final JSON and durable snapshots
in a newly created output directory. Reject malformed/oversized input and unexpected
results. Exit nonzero on failures. Windows GUI subsystem means no console window;
set idle priority before work, enforce finite steps and time budget. Hidden launcher
returns process ID/output paths rather than blocking the user's PC.

Validate debug/release core and runner cases, positive/negative scenario outcomes,
rollback, non-mutating previews, stale preview commit, mass cap, ID reuse after load,
malformed inputs and separate process reload. Check PE subsystem/priority/window
behavior without opening the game or moving input. No claim of power-loss testing
or visual engine capture. Local native compiler already available.

Defer stacks/partial transfers, equipment and game adapter until this lifecycle and
runner are reliable. Engine path inventory: InventoryBox::OnEvent mutates ownership
via GE_OWNERSHIP_TAKE/REJECT and UI notification; adapter needs authoritative event
completion and rollback/reconciliation, not a direct mirror call. GCC lab tests
cannot validate those MSVC/xrGame integration hooks. Document exact next gates.

## Implemented and validated

Shared command path now supports Move/Spawn/Erase/SetMass. Every command requires
a nonpersisted process-local session token plus revision; successful initialize/load
refreshes session, failed loads preserve it. Preview executes the same command on
a copy and commit always validates again. Schema2 persists ID high-water to prevent
reuse after committed deletion. Schema1 is explicitly rejected, with fresh fixtures
permitted by the user. No old-save compatibility wrapper.

Runner and hidden launcher use definitions/seed fixtures, expected command results,
assertions, checkpoint/load and cross-process resume. GUI PE subsystem, no console
attachment, IDLE process priority verified.28 core tests in each debug/release build
plus independent reload;19 runner integration cases including hidden PowerShell
launcher passed. Existing outputs are never overwritten; each run has JSON/JSONL
and snapshots. Logs and exact hashes live in milestone2 evidence alongside packages.

Read/reconciled review: C:/GAMMA/sqa-previews/inventory-next-20261027/next-milestone-review.md.
Epoch hazard addressed; stacked identity and equipment proposals deliberately remain
next stages. Current deletion rejects nonempty containers; no subtree destruction,
spill or crafting batch semantics are implied. Forced creation has no bypass.

Engine integration remains gated: InventoryBox.cpp:24-78 dispatches u16 object IDs,
H_SetParent, visibility/enabled state and callbacks on TAKE/REJECT. A core64bit ID
needs a persistent mapping/generation to these live IDs. World access may change
without core revision, so commit needs availability/reach checks and explicit native
event completion/failure reconciliation. Inventory.cpp:1038 mass and item save/load
hooks must be integrated with the SAME library after stacks/equipment/batch contract,
then built under MSVC and validated with real entities. None is certified by this
GCC headless test app. No speculative native hooks or game launch in this milestone.

Native no-argument entry now runs a finite default scenario directly; packaged users
can open the EXE without any script host/console. This entry, including paths with
spaces, passed integration checks. Windows Script Host refused the optional VBS
experiment with access denied; VBS was removed, not shipped or required. Hidden
PowerShell launcher also passed and provides the additional watchdog/hash manifest.
19 integration checks total; no visible window or game was launched for these runs.
