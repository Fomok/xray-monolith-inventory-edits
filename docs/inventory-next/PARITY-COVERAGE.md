# SQA3 implementation coverage â€” current increment

77 accepted parity rows retained. **None is certified complete in game.** This register is a progress handoff, not completion of the user request.

Source-backed requirements: C:/GAMMA/sqa-previews/inventory-next-20261027/SQA3-FEATURE-PARITY.md
Audit SHA256: dbf5f89d5f7546668f0184cdb58842c46064980200aa4301df119608e8e46654

Current increment: 117 core checks per debug/release build and independent ordinary/logical/world-subtree restarts pass. Loaded-world subtree acknowledgement/persistence foundations are offline-verified; native transfers remain OPEN. Dev15 purchases, ground/container/corpse admission and nested use have bounded native acceptance. Dev16 physical primary-slot-to-crate unequip and exact reload pass. Dev17 physical empty-primary direct equip, correct activation, context-menu occupied-slot refusal and exact reload pass; native-fixture carried-backpack rotation and boundary refusal also survive fresh reloads. Physical occupied-slot drag and R-key automation remain unverified. Dev18 loaded-container safety build0be12d07/CI40502480472 passes physical refusal UI, Move all subtree retention and exact reloads in the isolated lab. Ordinary direct-unequip regression and exact fresh reload also pass. Full77-row parity remains unfinished; detailed evidence below separates tested slices from open requirements.

Non-negotiable presentation requirement: one replacement inventory through the normal inventory key and normal loot/container paths; preserve useful native controls and semantics; **no3D player-model panel**. Diagnostic windows are not acceptance. Vanilla ui_inventory.script intercepts the normal entry; the scoped enable_feature guard retires this old Lua presentation only in SQA3 development mode.

| ID | Accepted behavior | Current evidence / remaining work |
|---|---|---|
| I01 | Ownership independent of visible panels | Core tree plus persistent native identities and authoritative ground/owned/purchase ownership acknowledgments pass bounded native fixtures; all producers and loaded-world subtrees OPEN. |
| I02 | Player grid, pockets and backpack sections | Native4x2pockets, carried8x8backpack and visible recovery pass physical transfers/reloads. Full authored catalog, rigs and section controls OPEN. |
| I03 | Backpack capacity and equip/remove transitions | Native backpack equip/remove, carried contents and exact reload pass; loaded drop safely refuses. Complete backpack catalog and world subtree transfer OPEN. |
| I04 | All native equipment slots, including knife and pistol | Native slots retained; pistol unequip/re-equip including rotated bag origin passes. Other slots/full parity OPEN |
| I05 | Equipment outside grids; grouped arrows | Core equipment and native slot/belt projection preserve mass outside bag. Pistol runtime equip/unequip and reload pass; grouped arrows and all-slot coverage OPEN |
| I06 | Capacity enforcement on all acquisition paths | Native ground, box/corpse Move/Take All and UI Buy batch guards pass full-storage refusal and exact reload; external equip/belt/use and other producers OPEN. |
| I07 | Full-grid policy and protected items | Strict ordinary pickup; explicit owned recovery overflow tested, no manual overflow capacity. Native forced-arrival/protected-item policy and per-container recovery OPEN |
| I08 | Manual placement and rotation persistence | Physical cross-storage drags and persistence pass; reload-binding rotation and edge refusal pass via native input fixture. Full physical keyboard/drag rotation coverage OPEN. |
| I09 | Valid/invalid/swap/overflow/new-item highlights | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| I10 | Finite stack ceilings and overrides | Per-type stack ceiling tested; actual catalog and context overrides OPEN |
| I11 | Stack member identity/state and partial transfers | Fungible type/state split/merge and stable stack IDs tested; heterogeneous per-member records/inspection OPEN |
| I12 | Hotfix stack footprint accounting | One tile per fungible stack with quantity mass tested; heterogeneous member footprints OPEN |
| I13 | Stack inspection picker | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| R01 | Rig compartment geometry and authored layout | Core compartments/masks tested; rig/pouch content/attachment policy OPEN |
| R02 | Equipped and spare-rig windows | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| R03 | Pouch fitting and granted storage | Stable mounted pouch ownership/grids, category/condition/slot checks and soft-host fullness tested; actual native mount/catalog integration OPEN |
| R04 | Occupied/worn pouch refusal and rollback | Nonempty unmount/new attachment to inactive mount refused; retained inactive pouch survives condition loss; native UI/events OPEN |
| R05 | Rig wear, damaged slots and loot condition | CORE: explicit condition reconciliation retains inactive pouch and recovers contents; see RIG-CONDITION.md. Native hit policy, authored tears, loot wear and gameplay OPEN |
| R06 | Pouch/rig upgrades and weapon state retention | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| R07 | Safe rig dismantle and pouch return | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| R08 | Healing/quick-use from rig pockets | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| R09 | Ammo access and reload gates | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| R10 | GAMMA/ZONA reload bridges | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| B01 | Box exterior vs interior/cell compartments | Core rigid exterior, distinct interior and compartments tested; actual boxes OPEN |
| B02 | Category-specific box acceptance | Core category masks tested; actual category definitions OPEN |
| B03 | Box stack limits distinct from inventory | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| B04 | Nesting policy conflict | Core acyclic authored nesting, recursive growth tested; content-specific allowed nesting policy OPEN |
| B05 | Box/rig live ownership and asynchronous transfer | Native box/corpse acquisition preserves source on refusal and reserves until client acknowledgment; rigs and loaded-box subtree transfer OPEN. |
| B06 | Recursive weight incl contents | Core recursive mass and native item Weight mapping agree in6/71item fixtures and reloads; broader content/nested-world catalog OPEN. |
| B07 | Persistent box layout/contents/bolts | Core schema4 roundtrip; game entity/bolt serialization OPEN |
| X01 | Direct equip from container with full backpack | Dev17 physical empty-primary take, correct active weapon, context-menu occupied-slot refusal and exact reload pass. Occupied-slot drag input, all-slot behavior and swaps remain OPEN. |
| X02 | Direct unequip into rig/box/stash | Dev16 physical primary-slot to native crate passes with full carried storage; exact71records, both weapon states and fresh reload preserved. Finite target capacity, rigs and loaded-container subtrees OPEN. |
| X03 | Same-grid/cross-storage swap | Final-state multi-relocation same/cross-container swaps tested, including unequal sizes/rollback; drag route OPEN |
| X04 | Quick store routing and feedback | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| X05 | Quick open, context actions, keyboard controls | Nested carried bread right-click/Eat and exact removal/reload pass. Quick-open and broad keyboard/custom-action parity OPEN. |
| X06 | Sounds, hover opening, pinning and drag feedback | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| X07 | Inventory vs trade filtered views | Normal inventory, native trade/loot views and hidden-item collision checks pass; complete filter/settings compatibility OPEN. |
| L01 | Corpse equipment and pockets layout | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| L02 | Looting Takes Time compatibility | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| L03 | Corpse numeric-ID/object normalization | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| L04 | Corpse rearrangement/rotation/save | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| T01 | Fixed stash capacity and exclusions | Core fixed-root grid/caps available; stash definitions/exclusions OPEN |
| T02 | Stash placement/stack/split/reservations | Core placements, quantity split, masks and persistence; pending reservation/lifecycle OPEN |
| T03 | Stash swaps and equip routes | Core swaps/equipment routes; actual stash entity route OPEN |
| T04 | Stash-only sorting, no loss or poorer packing | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| T05 | Marker cleanup and take notifications | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| T06 | Weapon attachments in stash | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| T07 | Safe stash disassembly | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| T08 | Stash save/travel/task-spawn lifecycle | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| U01 | Stats layout toggle, labels, gauges and tooltips | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| U02 | Food/consumable effect preview | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| U03 | Layout/scale/opacity/icon compatibility | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| U04 | Settings and localization | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| U05 | Optional wearable-device inventory | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| D01 | Existing item catalog and addon registration | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| D02 | Crafting and dismantle recipes | Ordered atomic consume/create batch tested; recipes/dismantle/native producers OPEN |
| D03 | Repair/upgrades/consumables mutate item state | Native nested bread use removes exact identity/mass and preserves all survivors after reload. Repair/upgrades/custom consumables OPEN. |
| D04 | Trade/new-game/death/treasure/reward/script spawn | Ground/owned acquisition and batch UI purchases verified including money/refusal/reload; other trade callers, rewards, death/new-game/treasure producers OPEN. |
| D05 | Save schema and engine reconciliation | Schema6native chunks, stable identities and exact repeated fresh-process item/money reloads pass bounded fixtures. Full travel/reconciliation/failure recovery OPEN. |
| K01 | Soft fullness-dependent exterior | Core stepped soft growth through all ancestors and equipped exception tested; game content/UI OPEN |
| K02 | Masks and separated compartments | Core masks/compartment boundaries, usable-cell fullness, rotation-only fit and persistence tested; content/UI OPEN |
| K03 | Nearby containers and floor before pickup | Observed source AccessGate revisions, read-only children and loss-of-access rejection tested; real discovery/native authority/UI OPEN |
| K04 | Independent scroll/collapse/reorder sections | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K05 | Grouped equipment browsing | Core stable-ID group cycling read-only; selection persistence and arrow widget OPEN |
| K06 | Queued transfers and reserved cells | Native accepted ground/owned/basket arrivals reserve cells; actual save gate blocks until client acknowledgment. General queued transfer UI/cancellation/reconciliation OPEN. |
| K07 | Multi-select, take-all, quick transfer and partial split | Core partial stack transfer, ordered batch and final-state layout transactions; multi-select/take-all policy/UI OPEN |
| K08 | Locked items and favorite-like protection | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K09 | Search/reveal and known-item state | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K10 | Sorting/layout templates/silhouettes | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K11 | Item details/search/filter/selection feedback | Native item tooltip, selection, inventory/trade filters and refusal text exercised; full search and selection requirements OPEN. |
| K12 | Visible overflow recovery | Visible actor recovery preserves existing displaced items and is not ordinary pickup capacity. Per-container recovery and every forced-output producer OPEN. |
| K13 | Persist layout/order/settings independently of world ownership | Core save plus explicit load invalidation; UI preference/layout template persistence OPEN |
| K14 | Item-definition editor and preset import/export | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K15 | Hotbar ordering, slot candidates and bindings | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K16 | Detailed item inspection | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K17 | Context action grouping and refusal reasons | Native nested bread Eat/Details/Drop menu observed; full-grid owned/purchase reasons visible. Grouping/localization/all custom actions OPEN. |
| K18 | Container-content amount transfer UI | Core split/merge quantity commands; amount selection UI OPEN |

## Next implementation order

1. Per-container stack policies, individual stateful member representation, atomic multi-item admission and forced-output recovery; do not silently delete overflow.
2. Rig/pouch mounting/granted storage, damage and removal rules; effective backpack/equipment domains and real item catalog.
3. Quick store, protected items, queued transfer lifecycle/reservations and view preferences; keep actor manual placement separate from requested stash sorting.
4. Engine entity/source discovery and authoritative event/save adapter; mapping u16 live IDs to durable IDs, access checks, event completion/rollback, mass mapping and every item producer/consumer.
5. Native interactive inventory views, scroll/collapse/selection/actions, localization/settings and upstream-compatible integration build. Validate with real game only under separate launch authorization.

The original standalone GDI tool remains a read-only diagnostic renderer. The current lab additionally runs the native replacement UI with FieldKit textures; see current increment above. Ordered batches validate each intermediate stage; simultaneous occupied-cell permutations use relocate(), which validates final state once. AccessGate currently guards single commands only; batches must not be exposed to untrusted world interactions before a guarded compound-transaction adapter exists.

Confirmed user policy27October2026 supersedes old blanket dismantling refusal: consume once, legally place outputs that fit, retain every remainder visibly in recovery. Core recipe transaction tested; native recipe hooks OPEN.


### 27 October: a3acc9d1 runtime regression

CI40451986671 succeeded; artifact12711617158; engine SHA2565143ef44dfe0be16da2cce1b52666615b1140b54d530f02d571227a9e24bda90. Corpse Take All now survives a temporary roubles identity (converted to money), transfers bread, and saves8items/2843g. Fresh reload retains all IDs, placement and mass; helmet condition1300->1294 is stock float_q8 serialization (ActorHelmet.cpp92 and CSE_ALifeItemHelmet UPDATE), so exact condition equality is not claimed.

Physical PDA drag during6201 condition-update ticks was rejected as stale_drag, displayed a refusal and preserved PDA4,3; fiveitems/1638g remain. A separate unchanged-state mouse drag succeeds to6,5. Native/model snapshot checks passed. Evidence under previews/inventory-next-20261027/ci-40451986671. Mouse bread-use remains unverified.

Next-stage core now accepts finite authored grids/masks and offers read-only ruck admission preflight.80 debug/release checks plus independent restart pass, including shrink/recovery, partial rows, rotation-only fit and failed admission atomicity. Native caller still uses the8x64 development grid: this is NOT integrated acquisition enforcement or final distinct pockets/backpack/rig storage. Server reservation/commit and explicit native recovery remain required.

### 27 October: rotation, context use and native capacity correction

92c91179/CI40455501979: physical drag selection followed by the native reload binding rotates the bag pistol; a right-edge rotation refuses with bounds. A fresh process preserves all five item records and1638g exactly. OS keyboard injection alone and drag-time ghost rotation remain unverified/open.

79017989/CI40457167495: native context opening plus physical Eat-menu click removes exactly one bread/190g. Fresh restart preserves four items/1448g with byte-identical export038f48e383391e18a83a677c211c2f3a781903465076195eb3bd0f9e37268275. This resolves the earlier native context-action uncertainty, not physical right-click/double-click shortcut coverage.

The first native capacity probe FAILED with overflow_owner: copied projected nodes retained grid coordinates while being staged in recovery.31d029b9 normalizes those coordinates before validation, retaining placement restoration afterward.87 debug/release checks and independent restarts pass, including this exact projected-position regression. CI40459553015 succeeds: native capacity preview now passes4x2 ->8x8 ->4x2 with no item/mass loss; repeated rotation survives rebuilds and filtering clears the hidden selection safely. Actual pickup enforcement and final separate logical storage are still OPEN.

Native sale through physical drag and Sell click passed on79017989 with stationary disposable NPC19200: bread18944 moves to the NPC, actor gains75RU and NPC loses75RU; final four items/1448g. Earlier sale fixture attempts were invalid because the NPC walked8.13m away and native trade correctly closed beyond3m. A fresh process buys that exact bread back through physical drag and Buy click: actor pays675RU, NPC receives675RU, native18944/core104 return, and all untraded item records remain identical. Final five items/1638g export457b610ba2a28b922f631d64bcad8b0e40f8166974d9cbb463cd8428bb897d29 matches the original replacement fixture. Test funds are granted explicitly; native strict admission remains open.


### 27 October: carried native storage acceptance

Corrected1a6d98fb/CI40468423845 passes native unequip/re-equip, five accepted physical mouse drags across two runs, and exact item-record preservation after fresh reload: six items/2458g. Pockets4x2, carried backpack8x8 and explicit recovery render with FieldKit textures. Loaded-drop refusal no longer blocks shutdown. See NATIVE-LOGICAL-STORAGE.md for exact evidence and remaining admission/world-transfer/rig limits.


### Dev12 native ground acceptance

4e556e9a/CI40473269081 succeeds. PID16648 admits exactly65 of67 ground bandages, with65 server/client acknowledgments and pending-save refusals. The two excess sources stay unowned across retries.71 carried objects/4928g preserve all six pre-existing records and place every ordinary arrival in Grid, never recovery. Fresh PIDs20468/28092 preserve byte-identical exportb43ec3a8f0df129cfe01277cc3d68be751608b0aa4f53b338521181d5c0606a8; the latter checks refused ground IDs19521/19522 and their mass/condition. PID13092 queues source destruction before pickup with room available: no acquisition, original six records unchanged. All quit cleanly. Final overlay0b0ed8e5; twelve payloads and original test save verified; no game running. Evidence and screenshot under ci-40473269081. This is actual native scripted pickup/ownership testing, not OS pickup-key testing. Corpse/container/trade admission and loaded world subtrees remain open.


### Dev13 staged mouse acceptance - 2026-10-27

Overlay8bf2df4a stages20s source refusal,30s normal inventory drop,30s reopened-source acquisition. Same cb5 engine. Dev13 ZIP superseded in place:10728077bytes SHA25663bef1427375f8c7a8cb169c67153aa870789354ccd7411e4fee956cfeaa13c1. PID23272 physical ground-area drag drops19777 from pocket2,0; physical source drag acquires19778 into freed cell; remaining19712 is refused once. Native server/client acknowledgment matches, all common records identical,71items4928g. PID13384 fresh restart preserves exact exportfcabb500b85116e9d033ec2df61fa650245d6cbebdd4436b35eff806f24fdeae and exact outside objects:19712parent19456,19777ground38gcondition1. owned-mouse.log/inv, owned-mouse-verification.json, owned-mouse-reload.log/inv/verification.json and screenshot under ci-40478895102. First two failed mouse attempts retained. OS keyboard drop remains unverified; successful drop used physical drag, not context menu. final-installed-save-audit.json verifies12payloads and original test.dds/scoc/scop byte-identical; elevated process check zero games. CI40482714703 for9bedebc6 status correction is still building.


### Dev14 nested context use accepted - 27 October 2026

Engine9bedebc6, overlay30b96748: PID20420 physical empty-space drag positions native cursor, right-click opens nested bread menu, left-click Eat removes exact104 from carried pack105. The Windows pointer alone had not moved the native cursor in prior failed attempts. Native snapshot/model verify5items2268g, all five survivor dictionaries unchanged. Fresh PID21136 reload preserves byte-identical export04ab2835cac593d62247dde8ee33d698f6ad238eb0439f83511ebfb5e1783295. Both exit0; evidence ci-40482714703/nested-use* and verifier855656d3. This resolves nested bread context use only; arbitrary custom actions and external-source use remain open. Purchase a6036dbe/CI40488824434 is still compiling, not runtime accepted.

### Dev15 purchase acceptance - 27 October 2026

Native67-refused/65-bought/2-refused fixture passes71items4928g and one73125RUsettlement; exact source/item/money reload passes. Physical mouse1-item Buy costs1125RU and preserves7items2496g after restart. Physical full-grid Buy refuses with selected basket retained, both sources and balances unchanged. Enginea6036dbe, CI40488824434, final overlayde40090d; source/evidence details and first failed fixture correction are in NATIVE-LOGICAL-STORAGE.md. All instances closed; original test save and12installedpayloads verified. This is purchase-path evidence, not full trade/sale parity.
