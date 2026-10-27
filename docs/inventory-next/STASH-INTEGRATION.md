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
