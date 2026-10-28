# SQA3 visual implementation plan — 28 October 2026

Authorized scope: implement the agreed replacement inventory presentation in
the isolated lab. Preserve the current ownership transactions and shutdown fix.
No public release, other personal engine, website edit or broader stash-parity
campaign. Keep the two existing untracked audit documents untouched.

## Measurements and materials

Authoritative starting point is released SQA2 source at
`C:/GAMMA/sqa-previews/release-2.1.0-hotfix1-full/source`, particularly
`gamedata/configs/ui/ui_inventory_16.xml`, its script-driven equipment layout
and texture descriptors. The screenshot and generated concept guide appearance
only; their pixels do not define game coordinates.

The XML uses a1024x768 canvas. Equipment panel width260.5; storage width382.
Equipment content begins at11,8. Helmet/backpack frames75x96, chest75x150,
small equipment75x42, long-weapon rows237x96, grenade/bolt34.5x46.
The released script's pistol override must be measured before replacing its
generic XML weapon-row dimensions. Preserve native item icon aspect ratios.

Place backpack below helmet, knife below chest, pistol below detector. Chest
armour/rig is one view area with small rectangular chevrons centered on its
vertical borders, never two visible chest frames. Selecting a view must not
equip, unequip, transfer or alter saved ownership. Preserve access to remaining
native equipment controls; do not add an equipment rail to the storage column.

Reuse exact Field Kit textures and descriptors. Build any missing header/tab/
chevron material deterministically from those assets, recording source hashes,
crop rectangles and output hashes. Never use an AI screenshot as a texture.

## Stages and acceptance

1. **Measured layout and assets.** Audit script overrides, map the retained
   native controls into the agreed equipment/storage/loot columns, produce the
   required atlas and a readable contact sheet. Check bounds, icon aspect ratios,
   header alignment and chest-button border centers before building.
2. **Storage presentation.** Center each storage grid. Add fabric/metal section
   bars with centered title, left collapse square and right actual contents
   weight. Hide empty recovery. Equipped storage stays integrated; carried
   unequipped containers open independent closeable windows. Keep persistent
   IDs/local grid coordinates separate from display offsets, collapse state and
   window position. Closing/collapsing must not move items or change capacity.
3. **Interaction integration.** Route drag, context action, rotation, selection
   and preview through the correct view owner. Valid/invalid placement feedback
   must use transaction preflight, not a second permissive placement policy.
   Never place hidden contents into an auto-growing fallback list.
4. **Nearby loot column.** Use labeled source tabs for ground items and each
   accessible stash, plus the explicitly opened corpse when applicable. An
   accessible empty stash remains selectable. Hide the column when no eligible
   source remains. Enumerate actual native objects with range/access checks;
   revalidate selection and each transfer. No proximity-triggered pickup.
   Keep corpse permissions and existing transaction/capacity refusals intact.
5. **Build and bounded lab acceptance.** Compile isolated branch, package exact
   engine/overlay hashes, verify all payloads and one-process guard. Test normal
   inventory entry, collapse/reopen, empty recovery, nested container windows,
   carousel without equipment mutation, placement feedback, source appearance/
   disappearance and mixed sources. Test representative transfer plus exact
   save/reload and clean shutdown. Record physical input limits separately.

## Integration risks to resolve during implementation

Current native display flattens every container into one scrolling list and
unconditionally adds recovery. Changing only XML would leave incorrect behavior.
The native adapter currently recognizes backpacks; full rig/pouch gameplay is
still unfinished. The UI must not invent equipped rig state or advertise mock
weights. Any missing native rig identity/control needs an explicit integration
decision and honest test coverage, rather than copying legacy serialized state.

Baseline: engine4e263457, overlay71e1cbd7, accepted shutdown buildCI40558659909.
Dev28 remains the verified rollback. Implementation and runtime acceptance of
this redesign are pending; this plan does not mark any parity row complete.

## Implementation checkpoint

The original extra pistol uses native binocular slot 5. Read-only provenance:
`C:/GAMMA/mods/113- Knife Pistol in Binocular Slot - RavenAscendant/gamedata/scripts/binoc_pistol_knife.script`.
Its compatibility explains the public modpack screenshot. Latest direct user
correction in Planning (28 October) explicitly defers a native third pistol slot:
keep only its visual frame below the detector for vanilla testing. No native
slot 17, weapon cycling change, duplicate item control or binocular replacement
is included. Both 237x96 long rows, knife and separate binocular control remain.
No installed source edited.

Opt-in `-sqa3-field-kit` now selects measured XML, exact public texture atlases,
centered/collapsible storage projections, nonempty-only recovery and independent
carried-container windows. Reserved native slot 16 supplies the shared rig view;
this does not add rig item definitions or finish rig/pouch gameplay. The chest
view toggle changes visibility only. Separate windows are bounded to four.

Core debug and optimized suite: 128 behavioral cases passed, including display
centering/collapse/separate views, cross-window moves and rejection of drops
into recovery-container ancestry. CI40611413303 compiled a335154b successfully.
Follow-up fixes and runtime visual acceptance remain pending. Nearby source
tabs, placement preview and movable windows are implemented but not yet
accepted in game. Full rig content remains outside this UI pass.
Do not call this checkpoint a completed redesign or player-ready release.

### Follow-up source checkpoint

Nearby tabs now enumerate eligible native ground items, accessible boxes and the
explicit corpse, retain empty boxes, revalidate identities/access on actions,
and use existing acquisition/world transactions. Ground pickup preserves native
callback/deny/visibility checks and does not occur on discovery. Headers use
actual source mass including hidden loaded descendants. Separate windows can be
moved by their title bars. Placement coloring uses the same DragSession preview
as drop commit; per-owner view caching avoids rebuilding a model projection for
every drawn cell. All these engine integrations await runtime acceptance.

CI40610107317 (44d7c737) failed: duplicate FieldKit.inl include and mixed integer/
float Fvector2.set overloads. Corrected in following source; no failed artifact
installed. Pistol remains visual-only under the latest direct user correction.

### Acceptance build preparation

Follow-up source preserves actor selection during unchanged empty-source scans,
keeps ordinary trade/living-companion projections, applies source filters and
readable labels, hides the third column when unavailable, and uses exact atlas
rims with compact unscaled status text. Recovery containers remain viewable
for retrieval but reject incoming drops. The bounded Field Kit fixture opens
a disposable save, allows 90 seconds of physical view testing, captures state,
saves a separate checkpoint and quits. No runtime acceptance claimed yet.


## User-rejected smoke and corrected asset integration

Dev29 (a335154b engine,19734729 overlay) opened the menu but is explicitly
rejected visually: stretched lining, metallic grid cells, standalone binocular
frame, unframed quickslots, text arrows and scrollbar overlap. User interruption
ended Computer Use and the owned process was closed; exit-1 is not clean-exit
acceptance. Original test save remains unchanged. This supersedes any impression
that a successful compile or texture-bounds test accepted that appearance.

Under the user's specialist split, Mod and UI authored final visual-assets-v1
at C:/GAMMA/sqa-previews/inventory-next-20261028/visual-assets-v1. Native integration
uses its exact three payloads and sqa3v1_ IDs. Provenance/build/verification sources
are retained under tools/inventory_lab/visual-assets-v1. The two DDS are pinned
through repo_relative entries in native-visual-assets.json; package creation reads
them from the pinned overlay commit, not uncommitted workspace bytes.

e05ff97d integrates complete panel artwork, plain grid with closing boundary
lines, measured equipment artwork and CUI3tButton state textures. Source tabs
reinitialize their scrollbar to the reduced viewport height. The shared utility
frame retains both vanilla equipment identities without ownership mutation; when
both are equipped their icons fit side by side. Chest tabs are14x20 centered on
x93/168. Quick targets remain native33x44 cells, with framed positions beneath guns.
Recovery headers sum actual subtree mass. CI40616582069 is the acceptance build;
corrected native screenshot/control/state/save/exit acceptance is still pending.
## Dev31 crash regression — 28 October 2026

Engine `b983a6df7a48c7551ec1786a4dd95d908185bac6`, CI `40621440409`
passed. Overlay `e87cc2759549f32a15f2623795976d9e699dabd1` adds a bounded
Field Kit regression and separate `sqa3_field_kit_regression` save.
The header background is now enabled after initialization.

The old drag preview entered `InventoryNextIdentity`, whose `can_save` call
can invoke Lua. Rendering overlaps the worker's Lua GC and map updates.
The preview now uses a pure existing-ledger lookup. Save and transfer
validation still use enrollment checks. This corrects an unsafe path;
the exact cause of the historical crash/freeze is not conclusively proven.

Actual lab test: PID10200, nine accepted drags (five automated pocket moves,
four subsequent user bag moves), collapse/expand and bag reopening. Automated
input paused when outside input was detected; the user confirmed finishing.
Save/exit0, no timeout. Fresh PID25204 reload/save/exit0. Both exports agree
exactly: seven items,3198g, identity/ownership/placement/condition unchanged.
No saveability warning/fatal or new Anomaly event1000; no game remains.
Original `test` hashes and pristine baseline remain unchanged.

Evidence: `C:/GAMMA/sqa-previews/inventory-next-20261027/ci-40621440409/`
(`regression-verification.json`, two run records/logs/screenshots and final
installed/save/process audits). Dev31 ZIP at
`C:/GAMMA/sqa-previews/inventory-next-20261028/Squared Away 3 Field Kit Dev 31.zip`,
SHA256 `78a06a971ab61a6a467f2684a975822b30ecfe42dd7b0f986df28c34c5c3fc8f`.
22 payloads verified and installed only in the lab. The available scene had
no nearby source panel; source interactions, exact nested-window movement,
long-session stability and full-mod acceptance remain unverified in this pass.
No public engine or release changed. Superseded Dev30/diagnostic downloads
removed with exact path/hash checks; Dev28 rollback retained.
