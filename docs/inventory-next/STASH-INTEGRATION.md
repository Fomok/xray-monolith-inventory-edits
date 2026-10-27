# Vanilla stash integration contract â€” source audit

Native stash integration is not implemented or gameplay accepted. Reference files were extracted from the
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
