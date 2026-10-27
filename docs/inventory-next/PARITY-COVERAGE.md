# SQA3 implementation coverage â€” current increment

77 accepted parity rows retained. **None is certified complete in game.** This register is a progress handoff, not completion of the user request.

Source-backed requirements: C:/GAMMA/sqa-previews/inventory-next-20261027/SQA3-FEATURE-PARITY.md
Audit SHA256: dbf5f89d5f7546668f0184cdb58842c46064980200aa4301df119608e8e46654

Current increment:78 core checks per build and independent reload. Native a193b694 replacement passes normal-key open/close/reopen, pistol unequip/equip, container Take All and exact5-item/1638g fresh reload. Physical mouse PDA drag and hidden-PDA collision refusal persist across restart. Native83a937b2 adds visible Field Kit cells and passes rotated-pistol equip and filter occupancy checks. Corpse Take All exposed a stale-layout collision; a3acc9d1 fixes synchronization/fallback placement and includes7cd29f89 stale-drag guarding, but their native build/runtime acceptance is pending. Mouse bread-use remains unverified; failed fixture assertions are not gameplay success.

Non-negotiable presentation requirement: one replacement inventory through the normal inventory key and normal loot/container paths; preserve useful native controls and semantics; **no3D player-model panel**. Diagnostic windows are not acceptance. Vanilla ui_inventory.script intercepts the normal entry; the scoped enable_feature guard retires this old Lua presentation only in SQA3 development mode.

| ID | Accepted behavior | Current evidence / remaining work |
|---|---|---|
| I01 | Ownership independent of visible panels | Core tree and read-only snapshots; actual engine ownership events OPEN |
| I02 | Player grid, pockets and backpack sections | Core blocked cells/partial rows and compartments; real pockets/backpack definitions and UI OPEN |
| I03 | Backpack capacity and equip/remove transitions | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| I04 | All native equipment slots, including knife and pistol | Native slots retained; pistol unequip/re-equip including rotated bag origin passes. Other slots/full parity OPEN |
| I05 | Equipment outside grids; grouped arrows | Core equipment and native slot/belt projection preserve mass outside bag. Pistol runtime equip/unequip and reload pass; grouped arrows and all-slot coverage OPEN |
| I06 | Capacity enforcement on all acquisition paths | Core Spawn/batches validate; actual item producers not hooked |
| I07 | Full-grid policy and protected items | Strict ordinary pickup; explicit owned recovery overflow tested, no manual overflow capacity. Native forced-arrival/protected-item policy and per-container recovery OPEN |
| I08 | Manual placement and rotation persistence | Native physical PDA drag6,4 survives fresh reload; hidden PDA blocks visible bandage overlap. Command-driven rotated pistol renders and equips correctly. Physical rotation control/full edge cases OPEN |
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
| B05 | Box/rig live ownership and asynchronous transfer | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| B06 | Recursive weight incl contents | Core exact recursive intrinsic-times-quantity plus descendants; native Weight mapping OPEN |
| B07 | Persistent box layout/contents/bolts | Core schema4 roundtrip; game entity/bolt serialization OPEN |
| X01 | Direct equip from container with full backpack | Core world/container-to-equipment direct route bypasses full grid; native event route OPEN |
| X02 | Direct unequip into rig/box/stash | Core Move unequips into validated destination grid; native event route OPEN |
| X03 | Same-grid/cross-storage swap | Final-state multi-relocation same/cross-container swaps tested, including unequal sizes/rollback; drag route OPEN |
| X04 | Quick store routing and feedback | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| X05 | Quick open, context actions, keyboard controls | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| X06 | Sounds, hover opening, pinning and drag feedback | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| X07 | Inventory vs trade filtered views | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
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
| D03 | Repair/upgrades/consumables mutate item state | Core mass mutation only; full state repair/upgrades/consumable integration OPEN |
| D04 | Trade/new-game/death/treasure/reward/script spawn | Core Spawn/batches only; native producers OPEN |
| D05 | Save schema and engine reconciliation | Standalone schema4 tested; engine serialization/reconciliation OPEN |
| K01 | Soft fullness-dependent exterior | Core stepped soft growth through all ancestors and equipped exception tested; game content/UI OPEN |
| K02 | Masks and separated compartments | Core masks/compartment boundaries, usable-cell fullness, rotation-only fit and persistence tested; content/UI OPEN |
| K03 | Nearby containers and floor before pickup | Observed source AccessGate revisions, read-only children and loss-of-access rejection tested; real discovery/native authority/UI OPEN |
| K04 | Independent scroll/collapse/reorder sections | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K05 | Grouped equipment browsing | Core stable-ID group cycling read-only; selection persistence and arrow widget OPEN |
| K06 | Queued transfers and reserved cells | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K07 | Multi-select, take-all, quick transfer and partial split | Core partial stack transfer, ordered batch and final-state layout transactions; multi-select/take-all policy/UI OPEN |
| K08 | Locked items and favorite-like protection | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K09 | Search/reveal and known-item state | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K10 | Sorting/layout templates/silhouettes | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K11 | Item details/search/filter/selection feedback | Structured ItemView and offscreen diagnostic render; actual tooltip/search/action UI OPEN |
| K12 | Visible overflow recovery | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K13 | Persist layout/order/settings independently of world ownership | Core save plus explicit load invalidation; UI preference/layout template persistence OPEN |
| K14 | Item-definition editor and preset import/export | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K15 | Hotbar ordering, slot candidates and bindings | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K16 | Detailed item inspection | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K17 | Context action grouping and refusal reasons | OPEN: retained requirement; no matching complete implementation or gameplay validation yet |
| K18 | Container-content amount transfer UI | Core split/merge quantity commands; amount selection UI OPEN |

## Next implementation order

1. Per-container stack policies, individual stateful member representation, atomic multi-item admission and forced-output recovery; do not silently delete overflow.
2. Rig/pouch mounting/granted storage, damage and removal rules; effective backpack/equipment domains and real item catalog.
3. Quick store, protected items, queued transfer lifecycle/reservations and view preferences; keep actor manual placement separate from requested stash sorting.
4. Engine entity/source discovery and authoritative event/save adapter; mapping u16 live IDs to durable IDs, access checks, event completion/rollback, mass mapping and every item producer/consumer.
5. Native interactive inventory views, scroll/collapse/selection/actions, localization/settings and upstream-compatible integration build. Validate with real game only under separate launch authorization.

The current GDI tool is a read-only diagnostic renderer; it does not implement game UI, live textures, input, camera capture or engine rendering. Ordered batches validate each intermediate stage; simultaneous occupied-cell permutations use relocate(), which validates final state once. AccessGate currently guards single commands only; batches must not be exposed to untrusted world interactions before a guarded compound-transaction adapter exists.

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
