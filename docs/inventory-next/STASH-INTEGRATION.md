# Vanilla stash integration contract â€” source audit

Native stash integration is a source candidate, not yet compiled or gameplay accepted. Reference files were extracted from the
isolated vanilla data into sqa-previews/inventory-next-20261027/vanilla-script-reference.
Production scripts remain unchanged.

`item_backpack.script` has two destructive creation paths: actor_on_item_use for
itm_actor_backpack and UICreateStash:OnAccept for the backpack context action.
Both create inv_backpack, record only the backpack section, then release the
original backpack. actor_on_item_take_from_box recreates an item from that section
when the stash empties. The script itself notes missing condition/upgrades.
Routing only one creation path would leave the other unsafe for nested storage.

The SQA3 replacement must preserve the original shell and descendants, so it must
not register the dedicated sqa3_ground_backpack in player_created_stashes or let
the vanilla empty-stash callback manufacture a replacement backpack. Existing
vanilla stashes need their original path until a separate migration is designed.

## Integration requirements

1. Preserve the native Create stash context action, naming/cancel dialog, map
   marker and useful callbacks. Intercept only the opted-in replacement path.
2. Resolve the original persistent item, validate ownership and the whole subtree,
   and use native ground provisioning plus the acknowledged world transaction.
   Do not call Lua drop_item as a substitute: its single-item reject path correctly
   refuses loaded storage, and does not create a container.
3. Extend explicit stash creation to empty backpacks too. Ordinary empty-item
   ground drop must retain normal semantics. Distinguish explicit stash creation
   from ordinary drop rather than treating every empty backpack as loaded.
4. Publish the named map marker and create callback only after client completion;
   timeout/refusal leaves the original shell untouched and no marker. Persist
   naming metadata with identity-safe lookup rather than assuming a live ID lasts
   across offline/online remapping. Keep pending saves blocked until consistent.
5. Retrieval must return the same backpack, condition, upgrades and full nested
   contents; marker removal follows complete empty-proxy cleanup. Do not award an
   additional bag from the vanilla callback. Retain source on capacity refusal.
6. Test both creation entry points, cancellation, empty/loaded/equipped bags,
   failed provisioning, full-pocket return, save/reload, map-marker persistence,
   offline/online and real travel. Existing corpse/ground tests are not substitutes.

Ground implementation currently accepts loaded bags only. New script/API glue,
empty explicit stash support in the engine, marker/callback integration and gameplay compatibility tests remain open.

## Persistent naming core — 2026-10-27

WorldSubtrees::name_stash stores a bounded name on the detached world-root metadata, keyed by persistent shell/custodian identities. Existing schema1 records remain readable. Native fact refresh preserves the name, while reacquisition excludes world-only metadata from carried items. LoadedStorageSession::prepare_release can stage the name before dispatch, so malformed/oversized text refuses without moving the shell or blocking saves. No native stash command or game script calls this yet.

Four new behavioral checks cover nested state and naming roundtrip, wrong custody/control-character/oversize atomic refusal, complete empty-shell release/reacquisition preserving condition/upgrades without duplication, and invalid naming before dispatch.125 checks pass in both debug and optimized builds, plus the existing independent ordinary/logical/world restart checks. Evidence C:/GAMMA/sqa-previews/inventory-next-20261027/native-stash-metadata/. This is offline core evidence, not a game Create stash acceptance.

## Native integration candidate — awaiting CI/gameplay

InventoryNextStashCreate captures an expected persistent identity from the dialog, accepts an empty or loaded carried backpack, and provisions the dedicated ground container. Ordinary empty-item dropping is unchanged. Native transaction preparation stages the bounded name before dispatch; the actor/world save gate stays pending through all client acknowledgements and the create callback. Nonserialized treasure markers are reconstructed from saved custody metadata, removed before native ID remapping/retirement and after empty-container cleanup. Pending transfers do not publish markers early. Names do not depend on recyclable live IDs.

The extracted item_backpack.script retains author comments and both vanilla fallbacks. With explicit -sqa3-stash-storage, actor_on_item_use and UICreateStash:OnAccept use sqa3_stash.script and three level bindings. The naming dialog captures persistent identity during Reset. Removal callbacks run after capacity preflight but before dispatch and may cancel without moving the bag. player_created_stashes is not populated, so vanilla pickup cannot create a duplicate backpack. Equipped-bag use moves that bag's logical subtree, not unrelated carried pockets/equipment. Source provenance is tools/inventory_lab/stash-source-provenance.json.

Packaging includes the two script files only when the pinned engine supports the stash mode; launcher -StashStorage enables its ground/world prerequisites and rejects unsupported manifests.125 debug/release core checks pass. Seven mocked Lua boundary checks pass in native-stash-metadata/lua-bridge-verification.json; this does not prove native bindings, marker rendering or gameplay. LuaJIT/Python/PowerShell syntax and git diff checks pass. Engine compile and physical empty/loaded/equipped/cancel/refusal/rollback/reload/travel tests remain required. No candidate has been installed yet.


## Compiled candidate and acceptance fixture - 2026-10-27

Engine fd20bc7b97698f59e55de14dd6043d2a23077c31 compiled successfully in CI40549072785/artifact12753609124. EXE26887168bytes SHA9a0cb09eb921f2b40b61beb5b950f3c9ec5169a2747181b0ad0faf088604330c. Overlay eb216d83 adds bounded native stash fixtures and strict loaded-stash verifier. Dev26 ZIP10810007bytes SHA3e0345220bfda1a4a062eb477a467ee6db1f3ffd2d9309c86974f478b1498e99;16payloads checked. Fixture uses normal naming dialog with a preset test label and physical accept/cancel or retrieval; equipped entry is a controlled script call, not physical use. LuaJIT/Python syntax passed. This is compiled, not yet gameplay accepted. Full parity remains unfinished.


### First native run blocked; converter correction building

Dev26 PID16348 timed out without current exports or a stash dialog; not accepted. Diagnostic overlayd20fb6e logs fixture registration, actor readiness and step2 before stalling. Source audit finds this luabind fork's string_class is luabind::internal_string (config.hpp98, luabind_memory.h70); policy.hpp only registers that string type as primitive. Returning std::string therefore selects unregistered-userdata conversion. Engine eb76e14adde1a5750915c0e65536aa030621cf40 changes only the Lua boundary to an owned internal_string, retaining decimal identity precision. CI40552310845 is building. The first run's failure cannot establish contents loss: no transfer was reached or accepted. Diagnostic Dev26 ZIP was replaced at the same path:10810046bytes SHA48c19a6069d0cfb1c8d3d994b2b7a5fd32967e65f671c88f4cbec1529f22b516. Previous hash is historical, not current. Runtime fix validation remains pending.


Overlay d566d3a0/fb29164f prepares a physical context-menu entry, callback cancellation and capacity refusal, actual map travel/marker checks and a strict empty-shell verifier against the retained corpse baseline. Syntax passed; these additions are not runtime acceptance. Superseded Dev24 ZIP and downloaded EXE were exact-hash checked and deleted (37660560bytes); cleanup manifest dev24-superseded-download-cleanup.json. Verified Dev25 rollback and all evidence remain. Original user test save hashes and pristine c57fe5b9 baseline were rechecked unchanged.


## Dev27 first native stash acceptance - 2026-10-27

CI40552310845 SUCCESS/artifact12757221312, engineeb76e14adde1a5750915c0e65536aa030621cf40, EXE26887168bytes SHA699eb2037503a1a5332f91eb479ca4acc2e7df28dcc1ea4faa4a6f6a34af6884. Overlay275d02116f778bfa2f4a2ffca93291b4563b543a, Dev27 ZIP10811120bytes SHA74fffe109d7b628108dc26a3c30493a32234e2465196b53b05d8b44687dc7771;16payloads verified before/after isolated install. Identity binding now returns decimal105 and the fixture progresses beyond the prior stall.

PID6836 normal backpack right-click -> Create stash -> naming dialog -> physical OK: PASS loaded creation, exact original shell105/nested106/medkit102, 3members2140g under dedicatedproxy336. Actor4/1138g remains exactf80ca0cccd9b61b816c73bd7235a6747a22bbc28b073cb996f264298fd8c1109. One post-client creation callback, saved name SQA3 field stash and marker present; worldSHA089a8820aa64d4bc101b1a160bb4038fbac85bdaa13952342c60d62d1e47e07d. Test label was preset by the fixture, not physically typed. PID8548 fresh reload PASS exact actor/world/name/marker and no callback replay. Both exit0/no timeout. Evidence ci-40552310845/stash-context-release and stash-release-reload. Initial verifier rejected the different fixture log format; d89fe305 explicitly selects the stash completion marker while retaining FAIL/crash, saved-game, clean-exit, custody and native/model checks. No game behavior was weakened to pass verification. Retrieval and remaining failure/lifecycle cases still pending at this checkpoint.


### Loaded roundtrip accepted; empty shutdown failure

PID35692 physical loaded-stash retrieval and PID15364 fresh return reload both PASS, exit0/no timeout. Actor7/3278g exactly matches initialized03ac30edb03ae8fab51bb61be7433b7e22b9e62ccb50f5fd89952cf05092855a; world emptyd424329db21a0d533be204a9b6ecf662a313f433305332524d97e3ecca6f1cee, proxy/marker removed, no reload callback replay.

Empty PID2640 created original shell1040/live1117 under proxy1034, one member and marker, then saved sqa3_stash_empty_release. Shutdown exit0xc0000005 means FAIL, not acceptance. Exact PDB/disassembly identifies AddOne's shared-string container access; candidate SDK lifetime fix4e263457 builds in CI40558659909. See SHUTDOWN-LIFETIME.md for confirmed evidence versus unknown caller. No further game run before assessment/build. Equipped fixture now opens inventory before ToSlot and asserts actual slot13 membership before use; LuaJIT syntax and seven mocked bridge checks PASS, native equipped acceptance pending. Failed Dev26 ZIP/EXE removed after exact hash/path verification,37697214bytes freed; Dev25 rollback and evidence retained.


### Targeted shutdown verification and explicit stop â€” 2026-10-28

Human scope narrowed in Planning to shutdown fix/targeted verification only, then STOP for visuals. Verified no interactive game running before install. CI40558659909 SUCCESS/artifact12760608010; engine4e263457ffcf085a5c0f7616bb69bbb6024e0aba, EXE26886656 SHA8ef21dc8776d86444df176252d448fbfe2594e3d3da4ed9ec86a99602d1e107d. Overlay71e1cbd705f30a5f22c1c2421f0cd6f0b458f638; Dev28 ZIP10810199 SHA2b56d0e1fcfc020387068c36c8e409b236f8a6c7786a4b67c531c48ff404e524,16payloads verified before/after isolated install/testing. PID31064 shutdown-empty-release: physical normal dialog OK, exact same actor/world as failed PID2640, native fixture/strict verifier PASS, save+exit0/no timeout. PID19084 shutdown-empty-reload: exact reload/no callback replay, save+exit0/no timeout PASS. Actorf80ca0cccd9b61b816c73bd7235a6747a22bbc28b073cb996f264298fd8c1109; world3b20e9f83a91293550c03bb5bb9c5e1cb0cbdf78903a3b6c6aef85fd1a899229. Both log SDK destruction before renderer teardown. final-process-event-audit.json:0games/0newAnomalyEvent1000 since first run. final-installed-save-audit.json verifies16payloads and original test.dds/scoc/scop unchanged; pristinebaselineclean c57fe5b9. This is bounded regression evidence, not proof of the historical caller or universal shutdown stability. Evidence ci-40558659909/. Superseded Dev27 ZIP10811120bytes removed with exact path/hash checks; dev27-package-cleanup.json. Keep Dev25 rollback and Dev27 EXE/PDB crash evidence. No public engine changes, no broader stash cases run. Work stopped per user; visuals next.
