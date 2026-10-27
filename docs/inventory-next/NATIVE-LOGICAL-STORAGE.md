# Native logical storage adapter

The opt-in `ActorStorage::logical` mode extends `ActorLayout` to retain an
actor-owned logical forest while native objects remain physically actor-owned.
The normal native caller still uses the Dev10 flat layout. This change is not
native pickup enforcement or a finished storage UI.

## Reconciliation contract

The caller supplies current native facts, each with physical actor parent 1.
Container definitions must be supplied explicitly from validated configuration;
the adapter never restores saved mass, condition or container capacity over live
facts. It restores only logical parent and placement. Equipment follows native
equipment state, including taking an item out of its logical bag when equipped.

Contents are restored deepest-first, before their outer bag's placement is
validated. This lets soft containers acquire their loaded footprint first.
Unequipping a loaded bag retains its descendants. A forced arrival or unequip
without a valid saved grid position remains in explicit actor recovery. Logical
mode never silently packs recovery into pockets. Missing or resized storage
leaves surviving actor-owned objects in recovery without losing their identity
or counting their weight twice. A surviving nested container retains its own
contents even when its outer storage is lost.

The persisted actor attribute `native_logical_storage=1` identifies this mode.
A flat caller rejects a restored logical forest instead of silently flattening
it. Native equipment overrides old logical membership; failed identity checks,
cycles and foreign destinations do not publish a partially changed model.

## Required before native activation

- Native ordinary pickup/loot/trade/drop must use the shared reservation and
  actual ownership acknowledgement boundary before any source detach. The
  recovery path is for forced reconciliation, never a full-grid pickup bypass.
- Dropping a loaded bag is a subtree transfer, not the missing-storage recovery
  case. Preserve the real descendants, their logical links and state. A native
  sync that observes only actor items cannot implement world subtree persistence.
- Storage and recovery panels must use logical parents. The existing single
  actor bag list cannot display overlapping coordinates from different parents.
- Reconcile native facts before action validation. Carried possession, manual
  actions/crafting and configured combat quick access need distinct queries.
  UI panel visibility is not an ownership or crafting permission. Dropped bags
  cannot enter ordinary actor crafting/use searches. See
  the local Mod/UI handoff `NESTED-ACTION-COMPATIBILITY.md`.
- Pending descendant actions must also protect ancestors against drop; cached
  identities, state and quantities need revalidation at mutation. Native weapon
  and external script paths remain unverified for logical mode.

## Validation on 2026-10-27

`native-logical-storage-build` under the isolated preview root passes 90
behavioral tests in debug and release, plus the existing independent-process
reload and a new independent-process native logical forest reload in each build.
Tests cover bag unequip/re-equip, native mass/condition updates, nested soft bag
ordering, forced equipment, missing/shrunken storage recovery, failed cycle and
foreign moves, and refusing flat-mode reload of a logical save.

These are core adapter tests. No new game launch, native mode activation,
ordinary pickup enforcement or gameplay compatibility is claimed.

## Native integration prepared after bda262d1

The explicit additional `-sqa3-logical-storage` lab flag connects current native
backpacks to authored storage definitions and saves the active logical model.
The original `-inventory-lab -dbg` requirements still apply. Without the extra
flag, the Dev10 flat caller remains in use. Logical saves must be loaded with it.

`NativeStorageView` projects each logical owner's local coordinates into labelled
sections of the native scrolling item list: pockets, each backpack, then red
recovery. Headers and nonexistent pocket cells are not legal destinations.
Mouse drop resolves the section back to the actual logical parent before shared
model validation. Native item cells, equipment and context actions are retained.
This is a bounded first native storage interface, not complete rigs/world storage.

Loaded backpack release is refused at UI drop/sale, manual-drop dispatch and the
server detach boundary until subtree world transfer is implemented. Scripted
forced destruction remains recovery reconciliation. Ordinary pickup/loot/trade
admission remains unfinished; this flag is an isolated test mode, not permission
to claim capacity enforcement. Action-aware rig restrictions and external
crafting integrations remain open.

The `-sqa3-fixture-logical` fixture starts from disposable `sqa3_replacement`,
creates one backpack, puts the real medkit inside, unequips the loaded bag and
tests a scripted drop refusal before saving `sqa3_logical_unequipped`.
`-sqa3-fixture-logical-reload` loads that save and re-equips it; optional
`-sqa3-fixture-hold` provides a bounded physical drag window. The verifier checks
all six actual item identities, native mass/condition, medkit parent/placement,
pocket assignment, native equipment and the refused drop. Corrected native runtime acceptance is recorded below.

91 debug/release core checks and both independent restart checks now pass. The
additional test verifies presentation coordinates round-trip to the correct
logical owner, headers/recovery do not grant capacity, and rendering does not
mutate ownership or mass.

## Physical ownership acknowledgments

`TransferSession::PhysicalOwners` now permits an explicit physical actor source
or destination while the command retains a logical backpack/pouch endpoint.
The override must be the endpoint's actor ancestor; foreign actors are rejected.
Every intermediate container and physical owner handle remains generation-checked
through dispatch and acknowledgement. A bag's native ID is not accepted as the
observed parent when the physical owner is the actor. Rejected release retains
the logical subtree, while mismatched post-dispatch ownership blocks saves until
reconciliation. Native producers must still verify actual source parent/state
before prepare and dispatch; this core extension is not native admission wiring.

93 debug/release checks and both independent restart checks pass in
`native-physical-owner-build`, including physical actor acknowledgment, refused
nested release, foreign-owner rejection and intermediate-ancestor disappearance.
The corrected native storage binary is pinned to1a6d98fb and includes this coordinator extension. Native admission producers remain unwired.


## Corrected native acceptance, 27 October 2026

CI40468423845 built1a6d98fb successfully. The earlier49e973de runtime failed at shutdown because a loaded-bag guard intercepted internal server teardown. The correction guards ordinary network release events while allowing internal rejection/destruction.

Five bounded native processes passed: unequip, fresh re-equip, three physical bag/pocket drags, two further medkit pocket/backpack drags, then an input-free fresh restart. All retained six real objects and2458g. Final pack105 is carried in pockets0,0; medkit102 remains inside105 at2,2. Exact item records survive reload, including native IDs, condition, mass, placement and equipment; whole-export hashes differ because of model metadata. Each fixture verifies loaded-drop refusal and clean quit. The verifier now supports --exact-reload with --previous to enforce that stronger restart assertion.

Corrected Dev11 overlayb5e06c83 uses the existing FieldKit atlas, with separately labelled pockets/backpack/recovery. Its twelve installed payloads and original test save were hash checked. Evidence: ci-40468423845 under the dated preview directory; stage1, carried and carried-reload logs/exports/verifications plus screenshots and final-installed-save-audit.json. No game remains running. This is carried storage acceptance only; strict acquisition admission, loaded world-bag transfers, rigs and full compatibility remain open.


## Ground acquisition, native build pending

4e556e9a adds -sqa3-strict-ground-pickup on top of the explicit logical/lab/debug modes. Ordinary GE_OWNERSHIP_TAKE is checked before OnTouch/attachment. The adapter uses GroundPickup/TransferSession, publishes the server-acknowledged logical placement, retains shared cell reservations until client Take, and refuses saves while acknowledgments are pending.95 debug/release checks and independent reloads pass; CI40473269081 and the67-source/65-cell native fixture are pending. Existing Dev11 remains rollback.

This flag deliberately does not intercept GE_TRADE_BUY: its corresponding sell may already have detached the source. Corpse/container/trade need producer-side reservation before detach, including UI refusal and monetary rollback. Auto-equipment currently needs a legal ruck reservation first; this conservative limitation and unsaveable currency items require further native compatibility work. GroundPickup constructs a transaction snapshot; native identity ledger remains the persistent ID authority. No ordinary admission uses recovery.


### Required owned-source integration boundary

`UIActorMenuDeadBodySearch.cpp::move_item_from_to` is used by individual moves, corpse Take All, box Take All and `SendEvent_Item2Ruck/Slot/Belt/Eat`. A failed reservation must be returned before UI `RemoveItem/SetItem`; Take All must refresh from native truth rather than clearing rejected source cells. `CScriptGameObject::TransferItem/TakeItem` also sends separate sell/buy packets and must use the same authority.

Trade has additional effects: `CUIActorMenu::OnBtnPerformTradeBuy` invokes `OnPerformTrade` before `TransferItems`; `CTrade::TransferItem` invokes on_before callbacks, sends SELL, changes seller money, sends BUY, changes buyer money and invokes trade callbacks. Preflighting only GE_TRADE_BUY would therefore lose both source and money semantics. Reserve the entire requested acquisition batch before these callbacks, revalidate callbacks before detach, and associate acknowledgment/compensation with exact identities. `Perform_transfer` is not a ready single-player replacement: its current callers are multiplayer and it does not call the single-player ALife graph detach/attach hooks.


### Dev12 native ground acceptance

4e556e9a/CI40473269081 succeeds. PID16648 admits exactly65 of67 ground bandages, with65 server/client acknowledgments and pending-save refusals. The two excess sources stay unowned across retries.71 carried objects/4928g preserve all six pre-existing records and place every ordinary arrival in Grid, never recovery. Fresh PIDs20468/28092 preserve byte-identical exportb43ec3a8f0df129cfe01277cc3d68be751608b0aa4f53b338521181d5c0606a8; the latter checks refused ground IDs19521/19522 and their mass/condition. PID13092 queues source destruction before pickup with room available: no acquisition, original six records unchanged. All quit cleanly. Final overlay0b0ed8e5; twelve payloads and original test save verified; no game running. Evidence and screenshot under ci-40473269081. This is actual native scripted pickup/ownership testing, not OS pickup-key testing. Corpse/container/trade admission and loaded world subtrees remain open.
