# Native world lifecycle — investigation in progress

## Verified lifetime cycles, 27 October 2026

Engine033e851c with overlayea67ff05 passed real ALife offline/online switching
for loaded ground bags (PID24396) and crates (PID14276), followed by physical
retrieval. Client objects disappeared while offline and returned online; all
three members retained their native custodian. Offline saving succeeded. Both
final exports preserve seven persistent items, nested placement, condition and
3278g, with an empty world ledger. The outer unequipped backpack's transient
native_slot reset13->0 after net re-spawning; no equipment/core state changed.
Verifier explicitly records this difference rather than claiming byte identity.
Both runs exited0. This does **not** establish area-transition safety or forced
server-ID reassignment: IDs happened to remain unchanged in these two cycles.

Evidence: C:/GAMMA/sqa-previews/inventory-next-20261027/world-lifecycle-dev22/
`ground-cycle-installed` and `crate-cycle`, with logs, actor/world exports,
run manifests and verifier JSON. Source verifier:
`tools/inventory_lab/verify_native_world_cycle.py`.

## Real travel failure reproduced

PID9500, engine033e851c, overlay678b6d1c, actually moved from fake_start to
l01_escape through native `jump_to_level`. At the destination the original
ground container still existed offline, but its three-member ownership list was
incomplete. SQA3 had blocked saving. The fixture reported FAIL and quit cleanly;
exit0 is not acceptance. Evidence `ground-travel.*` is retained. The final export
was written before travel, so it cannot establish destination contents.

The original source save remains unchanged; only disposable saves/automatic
saves in the isolated lab were used. Diagnostic overlayf318bd72/PID14340 confirms
the cause: game_setup.script:432 explicitly calls alife_release for all three
members during actor_on_first_update. Vanilla first-world setup clears every
stash when item_removal_done is false; fake_start skips that setup via IsTestMode.
The container itself survives. This is a never-initialized debug-save edge case,
not evidence that normal initialized-world travel loses these items.

Corrected fixture3eb9a5c2 first travels with all seven items carried, lets vanilla
setup finish in Cordon, returns to fake_start and saves a separate initialized
baseline. Travel tests now require item_removal_done before placing storage.
No vanilla cleanup override or invented replacement items were introduced.
PID35512 completed both initialization transitions and saved sqa3_world_initialized,
but subsequently crashed during shutdown (0xc0000005, EXE offset0x000c178e).
This shutdown fault remains unresolved; it is not counted as a clean run.
Fresh process31056 reloaded that save and matched its seven-item/3278g export,
then exited0. Vanilla initialization changed PDA condition10000->8100 and
pistol10000->7804; the initialized export is the comparison baseline.
PID38724 physically dropped the loaded bag from that baseline, saved and exited0.
Its ground custodian is live336/persistent1031. Original test and prepared saves
remain intact; prior ground/world source saves were copied before replacement.

Dev23 PID37900 then completed actual fake_start->l01_escape->fake_start travel.
All three members were offline at Cordon and online after return, with the same
custodian and live IDs. Its physical retrieval window expired, so fixture FAIL
is retained and this run is NOT full acceptance despite exit0.

PID28272 repeat completed physical retrieval after both actual transitions and
passed verify_native_world_travel.py: seven items/3278g, exact initialized state
except outer105 transient rucksack native_slot13->0; empty proxy removed, both
away and return saves successful, clean exit0. PID11096 fresh-process phase3
reload matched actor and world records exactly and exited0. Evidence under
ci-40530438260: ground-travel-retry and ground-travel-reload. Actor SHA256
68892c85f81c66281035c424e3787232dbc41ca77ee618cb58603147f755c3e3;
empty world SHA256d424329db21a0d533be204a9b6ecf662a313f433305332524d97e3ecca6f1cee.
The verifier also rejects the real incomplete PID37900 run. Server IDs remained
unchanged, so forced-ID-remapping still has no runtime acceptance.

## Separate travel gate candidate

Source477d889b (CI40530438260 succeeded, artifact12746134441) adds an inventory-save preflight to
CALifeUpdateManager::change_level and checks the transition save result. It
restores previous server command line and clears m_changing_level on failed SQA3
save after restoring actor/holder coordinates. This prevents progressing after
an unacknowledged inventory transaction or refused save. It compiled successfully;
pending-transfer refusal passed PID24940 with overlaya950edef. The physical drop
created a pending proxy, the fixture requested native jump_to_level, and the
engine refused travel. Timeout cleaned the proxy; all seven initialized actor
records and the empty world ledger remained exact. Saving succeeded, exit0.
Evidence ci-40530438260/pending-travel-block-retry; strict verifier flag
--travel-blocked. PID5440 failed to initiate its drag and is not acceptance.
The post-preflight save-failure branch is not yet fault-injected. This is not claimed to
fix the reproduced vanilla initialization cleanup. Engine SHA256:
6b06c235bdd8d958b0c89a807fa9bf4b73bfadd43e0b4c9b2de0ca60ae34706b.

No public engines or production files changed. Full loaded corpse storage,
vanilla stash integration, world destruction recovery and parity remain open.

## Crate travel manual-input blocker

Initialized crate release PID32984 passed physical drag, complete three-member
custody, strict export verification and save; exit0. Crate336/persistent1031,
actor4/1138g and world3/2140g match the initialized baseline.

Actual travel PID5448 reached Cordon but displayed Paused indefinitely. The
window remained responsive and mouse cursor moved; two tool Pause presses,
Escape and a focused click did not resume simulation. No return-trip or final
retrieval acceptance is claimed. Per the user's stop-on-manual-input instruction,
the owned process was explicitly stopped (exit-1, not supervisor timeout).
Evidence ci-40530438260/crate-travel.* is retained. The next manual check is a
physical Pause/Break key press after the same outbound transition; establish
whether it resumes before changing game code to bypass the pause.

All games closed at the final elevated process audit. All13 installed payloads
match the manifest and original test.dds/scoc/scop hashes remain unchanged;
final-installed-save-audit.json records this. Current tested package:
Squared Away 3 Travel Gate Test Dev 23.zip,10786556bytes,
SHA256decb6756b710aba274250e996a790bcaa0e1c889dcd5305a7761158a7add531b,
overlaya950edef0681be999db0a2b3ba1386419a4e7381. Three superseded Dev22ZIPs
were removed after exact-path/hash verification (32357382bytes), retaining all
evidence and current/rollback packages. This is a development package, not the
completed mod. No loaded-corpse implementation was introduced in this checkpoint.

## Independent continuation and offline-save restart

The later human instruction to continue independent work despite a blocked test
was verified in Planning. The paused crate run remains unverified and closed;
it does not block the separate corpse candidate or offline-save checks.

PID7800, engine477d889b/overlay4a429d72, loaded sqa3_cycle_ground_offline in a
fresh process. The custodian and all3 members were initially offline with no
client objects, then returned online and were physically retrieved. Strict
verify_native_world_cycle.py --resume-offline --ground passes; final7/3278g,
same original state except transient unequipped slot reset, empty world, exit0.
ci-40530438260/ground-offline-restart retains evidence. This extends restart
coverage, not the unverified crate area-transition result.

PID16228 repeats the fresh offline-save restart with crate19712: passes the same strict verifier and exact final records, exit0/no timeout. Evidence ci-40530438260/crate-offline-restart. Crate actual map travel remains separately unverified.
