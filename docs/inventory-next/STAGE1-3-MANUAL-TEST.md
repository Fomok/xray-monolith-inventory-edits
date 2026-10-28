# Squared Away 3 isolated manual test

Development instructions. Consult STAGE1-3-COMPLETION-CHECKLIST.md for actual acceptance; this file does not certify every step as passed.

Close any Anomaly instance. Run `Start-Squared-Away-3.ps1` in the isolated lab. It verifies the package, refuses a second game and opens the normal windowed main menu. Choose New Game and a starting loadout yourself, or Load a disposable `sqa3_*` save. The menu appeared in the automated check, but automated click/Enter did not activate New Game; fresh ordinary New Game initialization and loadout have not been verified. This is the specific remaining manual input. The original `test` save is a starting reference; save your changes under a new name. The debug F2 map remains available through the normal debug menu. Close the game after this test.

After loading, open the console and enter:

```text
run_string sqa3_test_items.give()
```

Close the console and wait a second. A test crate appears beside you. It contains fresh and worn rigs, three pouch sizes, two backpacks, medical/ammo/food cases, medicine, food, a pistol, ammunition and a Swiss knife. The kit does not equip anything, bypass pickup capacity or overwrite a save. Repeat the command only when you intentionally want another crate. You can also use the normal debug spawner with section IDs below.

1. Open the crate. Equip a Bandolier rig (`amprig_bandolier`) using its context action or drag to the Rig equipment view. The arrows on the chest frame switch Armour/Rig presentation. That switch must not equip an item by itself. The separate pistol frame remains visual only.
2. Put a small/medium pouch (`af_magpouch_s`, `af_magpouch_m`) in each mount. Double-click the rig or spare rig to inspect its original compartments and pouch bands. Check that adding/removing a pouch recenters the complete layout without moving logical items. A nonempty pouch must refuse detachment.
3. Put a bandage or medkit in an active rig/pouch compartment. Close inventory and try its quick-use key. Medicine carried elsewhere must not substitute for the packed instance; deliberate Use from an open inventory is allowed. Inspect the selected instance's condition/uses. Food is outside this medical rule.
4. Put ammunition in the worn rig or an active pouch and reload the pistol. Ammunition in a closed case or spare rig must not be selected automatically. Check rounds before/after and repeat with a partly used ammo box. External magazine-mod bridges are a later integration.
5. Try the medical, ammo and food cases (`ampbox_meds`, `ampbox_ammo`, `ampbox_food`). Allowed items must fit within a single authored compartment; wrong categories must stay at source. Portable storage cannot be put inside another portable storage interior. Actor pockets and world crate roots still accept containers.
6. Compare similar carried items using the context menu's Choose item submenu. It lists separate conditions, uses, rounds and upgrades. Selection must leave ownership and equipment unchanged. Operate on the selected member and verify the other members retain their state.
7. Try empty/partly filled/full backpacks and rigs, then rigid cases/pouches. Only backpacks and rigs change exterior footprint. Fill the intended destination before a pickup or manual unequip: refusal must leave the source intact. A rig swap may use the vacated legal pocket position; an outgoing item that does not fit must leave both participants unchanged.
8. Inspect the badly damaged Harness (`amprig_harness`, 5% condition). Its torn pockets and disabled mounts must not accept new contents. After forced loss of capacity, affected carried items must remain in explicit recovery. Repair must restore the authored layout without duplicating, deleting or automatically creating extra cargo capacity.
9. With the Swiss knife, try dismantling a loaded rig: it must refuse before tool wear or yield. Empty its own compartments and mounted pouches, then dismantle it. Empty pouches return as the same items, using recovery if pockets are full; the rig yields its original parts recipe and the tool degrades once.
10. Save as a new `sqa3_manual_test` save, quit, reopen using the same launcher and Load it. Check rig/pouch bindings, compartment positions, exact worn members, ammunition, charges and conditions. Close the game after testing.

Other representative sections: `equ_small_pack`, `af_magpouch_l`, `amprig_scout`. Native default item art comes from the Squared Away catalog; the unavailable original Kevlar world mesh is explicitly mapped to its supplied shared rig model. Growth stages are provisional configurable development balancing.

Report the failed step, item/section, source/destination and save name. Physical drag, keyboard rotation, stack-picker gestures, New Game flow and long-session behavior require their own native observation; a core/adapter test alone does not prove them.

The five backpacks now use new development compartment layouts: main, side, lower and base pockets with blocked lower corners. These are clearly marked SQA3 layouts, not claimed recovered SQA2 masks. All22 original authored rig masks remain unchanged. Local authoring/preset export can change these layouts.

### Backpack capacity audit
The new development layouts preserve original usable-cell counts: itm_backpack 64 to 64; equ_small_pack 80 to 80; equ_small_military_pack 96 to 96; equ_military_pack 104 to 104; equ_tourist_pack 120 to 120. A four-cell base pocket compensates for four blocked lower-corner cells; overall presentation gains one row. No usable-cell capacity reduction is intended. Compartment boundaries deliberately prevent one item spanning separate pockets, so some shapes may need rearrangement/recovery after a definition change. Growth thresholds remain the already disclosed provisional development balancing. native-backpack-layouts.json records original dimensions and both usable counts; import rejects unequal totals. All22 original rig masks remain unchanged.
