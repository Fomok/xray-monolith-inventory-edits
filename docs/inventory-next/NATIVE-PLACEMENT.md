# Native placement development slice

Only `-inventory-lab -dbg` enables this adapter. It projects actual actor-owned items into a development8x64 grid, with native equipment and belt items outside the grid. The normal actor menu owns equip/use/loot actions. Final pocket admission and recovery presentation are unfinished. Do not promote it as complete SQA3.

`inventory_next_probe export` reconciles native item facts and exports to lab appdata. `inventory_next_probe view` opens the native window. `inventory_next_move <core ID> <x> <y> <0|1 rotated>` validates through the same model as view controls. Escape closes. Layout persists in ALife chunk0x33514c59 alongside the identity ledger. Saved facts are never authoritative over current native mass/condition/size. Failed moves leave placement intact.

## Isolated runtime fixture

Use only the disposable lab. Preserve original test save, verify no game process before launching, use one windowed process and terminate that owned PID on timeout. `keypress_on_start off` avoids the startup input gate. Base arguments: `-dbg -inventory-lab -nosound -start server(test/single/alife/load)`.

The autorun script is inert without `-sqa3-fixture`. Add `-sqa3-fixture-spawn` to create bandage/medkit through actual ALife spawn; add `-sqa3-fixture-edit` to move100 to4,3 and attempt overlapping101 and negative bounds. `-sqa3-fixture-equipment` also spawns a pistol, moves it from its automatic slot to the bag, reopens the menu and equips it through native ToSlot. `-sqa3-fixture-loot` spawns a real inventory_box and bread, opens its native loot menu and sends Take All, then checks that exact bread identity belongs to the actor. Saves to sqa3_replacement and quits. Reload that save with `-sqa3-fixture-reload` (without spawn/edit/equipment/loot); export, screenshot, save sqa3_replacement_reload and quit. Compare native identities/mass/equipment and exported positions across processes. Do not launch concurrently. Automated domain commands do not establish physical mouse input success.

## Visual assets

The checked-in sqa3_native.xml contains nine slices from the existing Squared Away Field Kit controls atlas, with center aliased to native _back. Supply the unmodified atlas recorded in tools/inventory_lab/native-visual-assets.json. Do not copy legacy SQA scripts. Item atlas and rectangle come from each native item using vanilla icon rules. Native cell heading displays rotated icons.

## Validation

Current replacement evidence:87 core checks per debug/release plus independent restarts pass. Native31d029b9/CI40459553015 succeeds and passes finite-capacity preview growth/shrink without item or mass loss. Earlier native regressions verify normal inventory, equipment, real container/corpse loot, physical drag/filter/stale-gesture behavior and restart. Selected rotation through the native binding, edge refusal and restart pass on92c91179. Native context opening plus physical Eat click and a mouse-driven sale/fresh-process buyback pass on79017989. OS-keyboard-only injection and drag-time ghost rotation remain unverified/open. Selection rebuild/filter fixes in31d029b9 pass three consecutive native rotations and safely clear selection when filtering hides the pistol; all five items/1638g remain. The paragraphs below preserve earlier prototype evidence and are superseded where noted.

76 core behavior tests in debug/release and independent restart pass. Native acc4e204 CI40443443210 succeeded. Two fresh native runs accepted PDA100 at4,3, rejected overlap/bounds, saved and reloaded with identical export SHA256088dc85fde7461b1dfee2b73aad826970d01fcd9d038cfad3dc79b205e287943. Three real items retain native IDs, condition and638g. Real screenshot confirms Field Kit frame and item icons render. Physical input test remains inconclusive; do not treat command-driven movement as mouse verification. Prior4e9a11c0 runtime verified PDA identity save/reload and real bandage/medkit spawn,638g total, successful save/quit. This does not establish current layout chunk or visual behavior.

## Manual input gate

The lab has a guarded `Check SQA3 inventory controls.lnk` shortcut. It opens the disposable sqa3_layout save and the view automatically; click Select on the PDA, then Right. Check that the PDA moves one cell. The view pauses its fixture for60 seconds; the launcher closes its owned process after a180-second hard limit. No console typing is needed. Do not start another game while this test is open. The original test save is never overwritten. This remains a development test, not a finished mod.

## Replacement direction (supersedes prototype manual gate)

User confirmed Select PDA->Right. The normal CUIActorMenu is now the replacement controller in development mode. It loads SQA3 XML/Field Kit surfaces, retaining native drag/drop, equipment, context/use, trade and corpse/container behavior. The old parallel snapshot dialog is retired; probe view routes to normal ShowActorMenu. No3D player model. Actor development grid is now8x64; old16-column saves reconcile. Equipment/belt mass stays outside grid. Same-bag native dragging commits through core placement. Build/runtime verification of a193b694 pending. Full authored capacity/overflow/ownership parity is still open.

### Normal-key interception found and retired

Anomaly1.5.3 ui_inventory.script registered ActorMenu_on_before_init_mode and replaced GetActorMenu/hide helpers with UIInventory. Normal key23 therefore opened the Lua interface while CUIActorMenu::IsShown stayedfalse. The packaged source-derived script changes only enable_feature to disable that old presentation in -inventory-lab -dbg; outside that mode it remains enabled. Helpers remain present. This avoids merely adding another native menu behind the existing Lua frontend. Provenance/hashes are in native-visual-assets.json.


### 27 October: a3acc9d1 runtime regression

CI40451986671 succeeded; artifact12711617158; engine SHA2565143ef44dfe0be16da2cce1b52666615b1140b54d530f02d571227a9e24bda90. Corpse Take All now survives a temporary roubles identity (converted to money), transfers bread, and saves8items/2843g. Fresh reload retains all IDs, placement and mass; helmet condition1300->1294 is stock float_q8 serialization (ActorHelmet.cpp92 and CSE_ALifeItemHelmet UPDATE), so exact condition equality is not claimed.

Physical PDA drag during6201 condition-update ticks was rejected as stale_drag, displayed a refusal and preserved PDA4,3; fiveitems/1638g remain. A separate unchanged-state mouse drag succeeds to6,5. Native/model snapshot checks passed. Evidence under previews/inventory-next-20261027/ci-40451986671. Mouse bread-use remains unverified.

Next-stage core now accepts finite authored grids/masks and offers read-only ruck admission preflight.80 debug/release checks plus independent restart pass, including shrink/recovery, partial rows, rotation-only fit and failed admission atomicity. Native caller still uses the8x64 development grid: this is NOT integrated acquisition enforcement or final distinct pockets/backpack/rig storage. Server reservation/commit and explicit native recovery remain required.


### Capacity adapter preparation (next source, not enabled pickup enforcement)

`storage_catalog.h` parses rows-first sizes and legacy cell counts with blocked trailing cells; exact section overrides kind, then default. `export_native_capacity.py` produces sqa3_capacity.ltx from the audited rules and verifies the8-cell/4-column bare-pocket source defaults. Runtime `inventory_next_probe capacity` reads the actual BACKPACK_SLOT item and native kind, projects current native facts into a separate finite candidate, and exports sqa3-capacity-preview.inv. It does not change native ownership, active layout, carry limits or ordinary pickup acceptance. The test fixture equips/removes a real itm_backpack and keeps a rifle in the ruck to exercise no-fit retention.82 core checks/debug+release and independent reload pass; this adapter still needs its own native build/runtime run.

Final SQA3 remains distinct persistent pockets/backpack/rig logical storage, not this flat preflight projection. Native weapon compatibility can retain physical actor ownership while logical container ownership and recursive mass live in one model. Do not flatten stored contents on unequip.

Source admission findings: CInventory::CanTakeItem is called again inside Take after native ownership processing, so inserting only a capacity veto there would be too late. game_sv_Single::OnTouch is before server attachment for ownership events, but native/script transfers can already have detached their old source. Transfer admission must reserve against all outstanding events before source mutation, then acknowledge actual parent changes; failed trade also needs item/money rollback. The existing TransferSession primitive is not yet connected to those native paths. Forced/scripted arrivals must retain every object and mass in visible recovery. No producer is certified capacity-enforced by this preflight work.

Transfer coordination acceptance groundwork:84 debug/release tests and independent restart pass in native-admission-race-build. A shared TransferSession serializes two requests for the final cell; the second remains at source. Losing the source before dispatch cancels without mutation; after dispatch it blocks save pending explicit reconciliation. This is core evidence only, not a wired native pickup test. All panels must share one ownership coordinator; independent TransferSession instances do not share reservations.

Ground acquisition preparation: TransferSession now represents unowned ground with an absent source handle, only when explicitly requested and the logical WorldRoot has no registered native entity. Default container transfer still requires a live source; an offline known source cannot be disguised as ground. Rejection expects native parent0xffff, admission rechecks reachability, and actual native parent must be verified before prepare and dispatch by the future adapter.86 debug/release core checks plus independent restart pass in native-ground-admission-build. Native pickup wiring remains OPEN.

### Current native capacity result

The first79017989 preview rejected overflow_owner because copied projected items retained grid coordinates during recovery staging.31d029b9 clears staging coordinates and reapplies valid old placements afterward; the regression is covered by the87th core test. CI40459553015/artifact12714233431 engine SHA2567897120ad78536df542b8de930bedd390bbaa9527681cea0f2881b3290cf0942 passes the real rifle/backpack fixture: pockets4x2 legal1/recovery1/3720g; equipped backpack8x8 legal2/recovery0/4540g; backpack removed, pockets4x2 legal2/recovery1/4540g. All three native item records remain intact. The separate preview export is SHA256b69bbc36ebc47ba21673f3fae65343df22e15653c17f86f8a08d25a4efce62b2. This does not enforce native acquisition or replace the final logical storage forest.

### Context use and trade

The bounded context fixture selects bread by physical drag, opens the native menu via engine input and permits a physical click on Eat. Four remaining items/1448g survive fresh restart with identical export038f48e383391e18a83a677c211c2f3a781903465076195eb3bd0f9e37268275. Native sale and saved buyback retain bread native18944/core104, balance75RU sale and675RU purchase transfers, and restore the original five-item/1638g export457b610ba2a28b922f631d64bcad8b0e40f8166974d9cbb463cd8428bb897d29. Trade fixtures grant funds and hold their disposable NPC within the unchanged native3m range. They are not capacity-enforcement tests.
