# Squared Away 3 development plan for review

Squared Away 3 has a working native inventory foundation in the isolated vanilla lab. It can preserve nested item ownership and placement across several tested transfers, saves and map changes. It is not yet the complete replacement mod. The next work should turn that foundation into the full item catalog and everyday inventory interactions, then verify the supported modpack integrations.

This plan retains all 77 requirements in [the coverage register](PARITY-COVERAGE.md). The phases below are proposed for review; preparing this plan does not start them. The current header and container-window correction is the last active visual increment.

## What already works in bounded tests

- Normal inventory entry opens the replacement Field Kit interface. The current presentation uses Squared Away artwork, separate storage sections and carried-bag windows without a player model.
- Pockets and a carried backpack enforce finite space. Ground pickups, container and corpse acquisition, and a whole purchase basket have passed refusal and save/reload tests. Ordinary pickups cannot use recovery as extra capacity.
- Loaded bags can move between the actor, ground, crates and corpses in the tested scenarios. Interrupted transfers preserve ownership or roll back; selected offline, map-travel and fresh-reload cases passed.
- Direct primary-weapon unequip to a crate and equip into an empty primary slot passed. Context-menu refusal for an occupied slot passed. Physical occupied-slot drag and the physical R-key rotation path still need acceptance.
- Named loaded-stash creation and return passed. Empty-stash save and shutdown passed after a lifetime fix. Broader stash variations remain open.
- The latest completed regression preserved all seven items through repeated inventory moves, save, clean exit and a fresh process. This is bounded stability evidence, not long-session certification.

The background C++ tests also cover soft-container growth, compartments, pouch mounting, damage recovery, stack operations and compound transactions. Those capabilities still need real items, native controls and gameplay checks; they are not completed player features.

## Rules carried through every phase

Every item must have one authoritative owner, retain its individual state, and contribute its mass once. Looking inside a bag never transfers it. Each action must recheck access and the current item state before committing.

Ordinary pickup stays blocked when storage is full. Forced unequip, lost capacity and dismantling outputs may use visible recovery; recovery is never a manual storage destination. Dismantling consumes its inputs once, places outputs that fit and preserves every remainder.

Soft containers may change exterior size with fullness. Rigid containers keep their exterior and explicit restrictions. Nesting is governed by authored container rules, with no cycles and no unrestricted box-in-box assumption. Personal placement stays manual by default; stash sorting affects only the stash.

The current pistol frame is presentation only. A third native weapon slot is not part of this visual correction. Existing native equipment semantics remain authoritative. Public Squared Away 2.x, personal frame generation and old saves are outside this development package.

## Phase 1 Complete the current interface and safety baseline

Finish the current visual correction: actual actor name, faction, rank, portrait and money; truthful nearby-source information; a textured X on separate bag windows; no redundant collapse button inside those windows. Integrated storage sections may still collapse.

Then cover source switching, empty and disappearing sources, overlapping bag windows, scrollbars, collapse while a transfer is pending, selection, resize and supported screen proportions. Preserve the removal of Lua calls from rendering and verify repeated opening, movement, saving and shutdown.

**Acceptance:** one bounded visual/input run and an exact fresh reload for each changed behavior; no item mutation merely from browsing; no ghost selections or lost source access. Record physical-key tests separately from fixture-driven input. Obtain the user's visual review before calling the reference match accepted.

**Requirements:** I02, I08, U01, U03, K03, K04. The remaining work in these rows continues in later phases where content is required.

After visual acceptance, the first gameplay validation should resume the preserved stash regression list: equipped-bag creation, refusal cases and named-stash travel/reload. These are unfinished checks of existing work, before starting new catalog or gameplay implementation. Broader new stash features remain in Phase 4.

## Phase 2 Bring over the real item catalog and storage rules

Trace the effective Squared Away definitions and their dependencies, including inherited and patched values. Author explicit sizes, masks, compartments, category restrictions, stack limits, backpack grants, rigid cases and soft-container growth. Keep logical footprints independent of icon resolution.

Represent fungible quantities separately from groups of individually worn, loaded or upgraded items. Add a picker for individual stack members and preserve bolts and other special item records. Provide validated local definitions and preset import/export with reset and invalid-data handling.

Define a versioned SQA3 authoring/export contract for add-on definitions, with explicit validation errors and migration from supported older definition formats. Connect the corresponding ZoneBench export/import boundary in the later integration phase. This concerns authoring data, not migration of SQA2 game saves.

**Depends on:** the stable ownership and save model. **Acceptance:** every supported definition validates; allowed and refused nesting combinations behave correctly; full and partly filled bags survive shrink, growth and restart without losing items; stateful stack members retain condition, charges, ammunition and upgrades.

**Requirements:** I03, I10–I13, B01–B04, B06–B07, D01, K01–K02, K14.

## Phase 3 Complete equipment rigs and pouches

Integrate the real rig and pouch catalog, authored compartments, equipped and spare-rig windows, mounting, removal, upgrades and dismantling. Damage must disable the intended storage safely, preserve affected contents and restore usable storage after repair.

Complete all equipment routes and grouped browsing. Cover direct equip with a full backpack, direct unequip to each supported storage type, occupied-slot behavior, atomic swaps, and the distinction between cycling a displayed item and actually equipping it. Add quick-use and ammunition access rules before connecting external reload systems.

**Depends on:** Phase 2 definitions and stateful items. **Acceptance:** native tests for every supported slot and rig/pouch transition, including full targets, worn mounts, loaded removal, repair and reload. A failed action must leave all participating items unchanged.

**Requirements:** I04–I05, R01–R09, X01–X03, K05, K15.

## Phase 4 Complete transfers looting and world storage

Finish the transfer interface: quick store/open, multi-select, partial quantities, Take All, visible reservations, cancellation, feedback and clear refusal reasons. Preserve locks and access restrictions across every route. Add the full corpse equipment/pocket layout and rearrangement, plus fixed stash capacities, exclusions and stash-only sorting.

Extend the existing world tests to all supported containers and nested sources: lost access, source destruction, pending transfer, equipped-bag stash creation, offline transitions, travel, restart and task-generated stashes. Verify marker cleanup and pickup notifications.

**Depends on:** Phases 2 and 3 for actual content and equipment destinations. **Acceptance:** conservation of identities, state, ownership and recursive mass after success, refusal, interruption and reload; no stale source action; no hidden or locked item moved by a bulk command. Sorting must not lose items or worsen packing.

**Requirements:** I01, I06–I07, I09, B05, X04–X06, L01, L03–L04, T01–T05, T08, K06–K08, K12–K13, K17–K18.

## Phase 5 Connect gameplay systems and action providers

Integrate crafting, dismantling, attachments, repairs, upgrades, consumables, trade, new-game equipment, death, treasure, rewards and scripted spawns. Inventory rules must apply to all producers and consumers, including those that bypass the visible UI. Preserve native and external action dispatch without allowing a stale nested item to be used after it moves.

Cover GAMMA/ZONA reload bridges and use one compatible search/reveal policy with Looting Takes Time. Filters must change presentation without changing ownership or leaking unrevealed contents. Build compatibility adapters around documented boundaries instead of copying the old inventory UI.

Connect ZoneBench and add-on exports to the Phase 2 schema: validate supported fields and references, migrate supported definition versions, and verify exported packages in the lab. This is a bounded authoring integration, not a ZoneBench rewrite or a website publication step.

**Depends on:** the item/state schema and transfer rules from Phases 2–4. **Acceptance:** actual supported modpack scenarios for every integration, including failure and full-storage cases. Dismantling yields every legal output once; money and item state remain consistent; quick use and reload agree with current accessibility.

**Requirements:** R10, X07, L02, T06–T07, D02–D05, K09.

## Phase 6 Finish inspection settings and convenience features

Complete stats layouts, gauges and tooltips, food/effect previews, detailed item inspection, search/filter feedback, settings and localization. Add optional wearable-device access, explicit layout templates and silhouettes, and configurable controls. Finish sounds, hover opening and pinning without changing item ownership.

**Depends on:** stable actions and actual item data. **Acceptance:** changing a preference never moves an item, filters retain hidden ownership, details update after state changes, and layout/template restoration handles missing items safely. Verify supported languages, icon sets and screen scales with real captures.

**Requirements:** U02, U04–U05, K10–K11, K16, plus the remaining presentation portions of U01, U03, X06 and K13–K15.

## Phase 7 Qualify the complete development package

Run the entire coverage register against the selected vanilla and modpack targets. Include new games, repeated saves, travel, long inventory sessions, interrupted actions, death and restart. Check performance and memory over repeated cycles, not just one launch. Verify dependency provenance and packaging, document supported configurations and retain a known-good rollback.

**Depends on:** all preceding phases. **Acceptance:** every one of the 77 rows has implementation references and appropriate native evidence, or an explicitly reviewed limitation. A green core suite alone cannot close a gameplay row. Public release remains a separate user decision.

## Choices to review before the next gameplay phase

1. **Compatibility order:** proposed sequence is isolated vanilla first, then GAMMA, then ZONA, with a named supported version for each.
2. **Search and reveal:** decide whether to retain the existing Looting Takes Time behavior or replace it with one equivalent native policy; never run two competing timers.
3. **Container balancing:** approve the per-container dimensions, fullness stages, limits and allowed nesting combinations when the real catalog is presented. The data-driven nesting policy itself is already decided.
4. **Bulk transfers and convenience:** settle whether mixed-validity selections move only the legal subset or refuse as a whole, what locks protect against, and which gestures trigger quick actions. Show the consequences before committing a batch.
5. **Optional access and amounts:** define which wearable devices gate inventory access and which real fluid/charge systems need amount-transfer controls. Keep unsupported chemistry or unrelated game systems out of scope.

These choices do not reopen the agreed recovery policy, manual placement, native ownership, reference artwork or current visual-only pistol frame.

## How progress will be reported

Each phase should deliver a separate test package, a short behavior summary, exact tests and remaining manual checks. Keep one isolated game instance at a time and close it after each bounded run. If a necessary test cannot be automated, state the exact manual action and expected result; do not label it passed or repeatedly retry unchanged input injection.

The [technical register](PARITY-COVERAGE.md) preserves all requirement IDs and evidence. Its historical entries describe their own checkpoints; the current summary and row status take precedence over old statements that a later-tested path was still open.
