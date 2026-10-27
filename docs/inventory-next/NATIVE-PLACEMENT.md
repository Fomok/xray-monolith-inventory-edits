# Native placement development slice

Only `-inventory-lab -dbg` enables this adapter. It projects actual actor-owned items into a development8x64 grid, with native equipment and belt items outside the grid. The normal actor menu owns equip/use/loot actions. Final pocket admission and recovery presentation are unfinished. Do not promote it as complete SQA3.

`inventory_next_probe export` reconciles native item facts and exports to lab appdata. `inventory_next_probe view` opens the native window. `inventory_next_move <core ID> <x> <y> <0|1 rotated>` validates through the same model as view controls. Escape closes. Layout persists in ALife chunk0x33514c59 alongside the identity ledger. Saved facts are never authoritative over current native mass/condition/size. Failed moves leave placement intact.

## Isolated runtime fixture

Use only the disposable lab. Preserve original test save, verify no game process before launching, use one windowed process and terminate that owned PID on timeout. `keypress_on_start off` avoids the startup input gate. Base arguments: `-dbg -inventory-lab -nosound -start server(test/single/alife/load)`.

The autorun script is inert without `-sqa3-fixture`. Add `-sqa3-fixture-spawn` to create bandage/medkit through actual ALife spawn; add `-sqa3-fixture-edit` to move100 to4,3 and attempt overlapping101 and negative bounds. `-sqa3-fixture-equipment` also spawns a pistol, moves it from its automatic slot to the bag, reopens the menu and equips it through native ToSlot. `-sqa3-fixture-loot` spawns a real inventory_box and bread, opens its native loot menu and sends Take All, then checks that exact bread identity belongs to the actor. Saves to sqa3_replacement and quits. Reload that save with `-sqa3-fixture-reload` (without spawn/edit/equipment/loot); export, screenshot, save sqa3_replacement_reload and quit. Compare native identities/mass/equipment and exported positions across processes. Do not launch concurrently. Automated domain commands do not establish physical mouse input success.

## Visual assets

The checked-in sqa3_native.xml contains nine slices from the existing Squared Away Field Kit controls atlas, with center aliased to native _back. Supply the unmodified atlas recorded in tools/inventory_lab/native-visual-assets.json. Do not copy legacy SQA scripts. Item atlas and rectangle come from each native item using vanilla icon rules. Native cell heading displays rotated icons.

## Validation

Current replacement evidence: a193b694/CI40447241129 passes normal-key inventory, equipment, real container loot and exact5-item/1638g restart. Physical PDA drag to6,4 and hidden-PDA overlap rejection are logged and survive fresh reload.83a937b2/CI40450044564 passes the visible-grid, filtered-occupancy and rotated-pistol equip fixture. Corpse Take All found an occupied-cell assertion after a transient identity sync failure. a3acc9d1 and7cd29f89 correct stale placement and gesture guards; native retest pending.78 core checks per debug/release plus independent reload pass. Mouse bread-use is unverified. The paragraphs below preserve earlier prototype evidence and are superseded where noted.

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
