# Native logical storage adapter

## Current checkpoint — 2026-10-27

121 core checks per debug/release build and independent restarts pass. Dev15 purchases, ground/container/corpse admission and nested use have bounded native acceptance. Dev16 physical primary-slot-to-crate unequip and exact reload pass. Dev17 physical empty-primary direct equip, correct activation, context-menu occupied-slot refusal and exact reload pass; native-fixture carried-backpack rotation and boundary refusal also survive fresh reloads. Physical occupied-slot drag and R-key automation remain unverified. Dev18 loaded-container safety build0be12d07/CI40502480472 passes physical refusal UI, Move all subtree retention and exact reloads in the isolated lab. Ordinary direct-unequip regression and exact fresh reload also pass. Dev19 physical nested actor/crate roundtrip, Take All, full-pocket refusal and exact reloads pass. Dev20 verifies partial native rollback in both directions and fresh reloads. Installed Dev21 engine033e851c/CI40520705952 passes12 bounded ground-bag runs: physical drop/retrieval, full-pocket refusal, timeout, partial rollback both ways and exact reloads. See docs/inventory-next/GROUND-BAG-ACCEPTANCE.md for evidence and remaining limits. Full77-row parity remains unfinished; detailed evidence below separates tested slices from open requirements.

## Initial adapter contract (historical starting point)

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


### Dev13 owned-source native acceptance - 2026-10-27

Engine cb5cb4619edca84f9ed8164ec3450483b512dfb5 CI40478895102 SUCCESS artifact12722559100, EXE26809344bytes SHA25675999d6dc16aa0c5a9e9412b132ba17d3268219370dbdabf188022075deefaa1. Overlay c6e9b929a70ed6015caa4228a059a9d508a1782f. Squared Away 3 Owned Admission Dev 13.zip10727098bytes SHA2561be62e97a640a8570a99b03876c3a03a4c65460b0d81e4a5b33867d136c3a875;12installedpayloads verified. Dev12 rollback retained.

Evidence C:/GAMMA/sqa-previews/inventory-next-20261027/ci-40478895102. PID38392 owned-box and PID13088 owned-box-reload PASS:65accepted,2retained at19456 (19712,19778),71items4928g, all six baseline records preserved,65server/client acknowledgments and pending-save refusals; exact export f28fba12de757eabc99a05af8846650f98562c76cbd162435430d3ef0f9bf06f survives reload. PID18984 owned-corpse and PID14392 owned-corpse-reload PASS: same counts, retained19968/20034 at19456, exact export83ab35bb2c457c53faaae090c3c0c21c0df9f3b4e3f5b90ee07e4d7d260271aa. All four quit0 naturally; corpse source list visually inspected on reload.

Mouse attempts PID16484 and13860 log actual full-grid refusal, but fail fixture acquired=1 assertion: no actor item dropped/acquired. Preserve owned-mouse-attempt1/2.log as failed test evidence, not ownership corruption. Loot context menu has no Drop option; G is bound and native handler exists but the attempted sequence did not complete. Fixture now separates refusal, normal-inventory drop, and reopened-source acquisition phases. Actual status-label defect: synchronous u_EventSend refusal was overwritten by the subsequent Checking storage label. Commit9bedebc6 moves the pending label before dispatch; CI pending, not installed yet. Native trade/external equip/use/loaded world subtrees and unusual client-rejection reconciliation remain open.


### Dev13 staged mouse acceptance - 2026-10-27

Overlay8bf2df4a stages20s source refusal,30s normal inventory drop,30s reopened-source acquisition. Same cb5 engine. Dev13 ZIP superseded in place:10728077bytes SHA25663bef1427375f8c7a8cb169c67153aa870789354ccd7411e4fee956cfeaa13c1. PID23272 physical ground-area drag drops19777 from pocket2,0; physical source drag acquires19778 into freed cell; remaining19712 is refused once. Native server/client acknowledgment matches, all common records identical,71items4928g. PID13384 fresh restart preserves exact exportfcabb500b85116e9d033ec2df61fa650245d6cbebdd4436b35eff806f24fdeae and exact outside objects:19712parent19456,19777ground38gcondition1. owned-mouse.log/inv, owned-mouse-verification.json, owned-mouse-reload.log/inv/verification.json and screenshot under ci-40478895102. First two failed mouse attempts retained. OS keyboard drop remains unverified; successful drop used physical drag, not context menu. final-installed-save-audit.json verifies12payloads and original test.dds/scoc/scop byte-identical; elevated process check zero games. CI40482714703 for9bedebc6 status correction is still building.


### Dev14 accepted and batch purchase candidate - 2026-10-27

Installed engine9bedebc6072560c03ebcec617212006eddd91949 CI40482714703 SUCCESS artifact12725330192,26809344bytes SHA2560cdeb429e153afb31f9cbe221bbb88109993f7e444c8e28deff46ade6fd2e1ff. Overlay19286a7b. Squared Away 3 Owned Admission Dev 14.zip10728941bytes SHA2568b051d07805e9352cf84b5c4671283abd8d58d0e4747c65309cc621a66cd4ecc. PID10604 full container regression passes65accepted/2retained,71items4928g,65acks and pending-save gate; screenshot visibly reads No space. Item stayed at source. Evidence owned-box-status.log/inv/verification.json/jpg under ci-40482714703, exportf28fba12de757eabc99a05af8846650f98562c76cbd162435430d3ef0f9bf06f. PID6556 prepare-only save sqa3_nested_manual succeeds; bread104 logicalparent105 position3,3, all other baseline records unchanged,6items2458g. nested-manual-prepare.log/inv/verification.json; final-installed-save-audit.json verifies12payloads and original test.dds/scoc/scop. Both quit0; elevated zero-game check. Manual nested-use GUI remains unverified, two failed input attempts preserved under ci-40478895102.

Purchase candidate commits63f3345e then30f5dd50619a30e58cdfdf97d480ce21fd2ee2f6. New purchase_session.h prepares a complete candidate without live-model mutation, validates money arithmetic and carried destinations, checks layout/session/registry generations, requires exact all-or-none native parents, and withholds publication until observed payment matches. Partial outcomes or incorrect settlement block saves pending reconciliation.104debug/release checks plus both independent restarts pass in native-purchase-build (linked unchanged ground core libraries; new coordinator is header-only).

Opt-in -sqa3-strict-trade-pickup implies owned/ground guards. UI Buy gathers every selected stack member before old RemoveItem/money code. Request preflights, preserves OnPerformTrade/on_before_sell/on_before_buy callbacks, then server validates identities, current native facts/prices, seller trade partner/liveness/distance and money. GE_SQA_PURCHASE performs silent ALife detach/attach for the complete basket; failure rolls prior members back before client broadcasts. Successful ownership acknowledgment precedes money settlement and logical publication/reservations; SELL/BUY notifications then reconcile client Take. End-of-Take callback waits for all members, refreshes trade lists, and invokes trade callbacks on exact native identities. Purchase blocks saves and other releases/acquisitions while pending. Source/fixture changes compile pending, not installed. Existing public engine and production files unchanged.

Native risks to validate: compile/API correctness, full67-item refusal with money/source unchanged,65-item acceptance and one settlement, repeated full retry, exact save/reload and physical trade Buy button. Runtime reconciliation after unusual client failure, arbitrary callback mutation/rollback, sales, external equip/use and loaded world subtrees remain open. The next fixture drives the same purchase authority from a diagnostic command, not a physical UI purchase claim.


### Dev14 nested use acceptance and purchase review - 2026-10-27

Engine9bedebc6072560c03ebcec617212006eddd91949 unchanged; installed overlay30b967487a8e5c187efc3b6bcf9fa61f836fe540. Dev14 ZIP superseded in place:10731231bytes SHA256f22b63df8b1ecab74989cc373bad90f9d6f83d53b9268339b21208933655762e. PID2500 attempt3 observed real Eat/Details/Drop menu but timed out before consumption; nested-use-attempt3.log retained as failed assertion. Observed native cursor remained near center despite OS pointer warp; empty-space drag positioned native cursor, then right-click opened menu. This is input observation, not a proven general OS/backend diagnosis.

PID20420 with longer90s input window: physical empty-space drag, right-click bread104 inside pack105 position3,3, then left-click visible Eat. Bread disappeared and UI mass fell. Native fixture verifies5items2268g; verifier855656d3 checks exact removal104 and all other node dictionaries unchanged against nested-manual-prepare.inv. PID21136 fresh sqa3_nested_use reload passes exact records and byte-identical export04ab2835cac593d62247dde8ee33d698f6ad238eb0439f83511ebfb5e1783295. Evidence ci-40482714703/nested-use.log/inv/verification.json and nested-use-reload.log/inv/verification.json. Both exited0; elevated zero-process check. final-installed-save-audit.json verifies12payloads plus original test.dds/scoc/scop against pre-test-save. No general custom-use/external-use acceptance.

Purchase native commit a6036dbe0e4b62629afc8c35bd1ca3f3443b7cf9 CI40488824434 still compiling. Supersedes30f5/CI40487812244 and63f3/CI40487603138, cancelled without downloads. Review fixed SqaPurchaseRefresh ordering because InitInventoryContents clears every list: actor first, seller second, restore queued sales by native IDs. Requests bounded512; seller/access checked before callbacks. Tool overlaycca9090e adds separate90s mouse Buy fixture with one exact seller bandage and fresh reload money/identity checks; not yet executed. Existing scripted67-refused/65-bought/2-refused fixture remains pending. 104debug/release core checks and independent restarts were passed previously; no new native runtime trade acceptance yet.

Cleanup manifest C:/GAMMA/sqa-previews/inventory-next-20261027/dev11-dev12-superseded-zip-cleanup.json records exact hash/path verified deletion of only Dev11/12 ZIPs,total21417691bytes. CIEXEs, source, saves and evidence retained; Dev13rollback and Dev14package retained. Previous rejected rollback deletions were not retried. Only isolated lab reads/writes plus source FieldKit atlas from fullHotfix1; public/pristine/production unchanged.


### Dev15 purchase native and mouse acceptance - 2026-10-27

Enginea6036dbe0e4b62629afc8c35bd1ca3f3443b7cf9 CI40488824434 SUCCESS artifact12729067471, EXE26860544bytes SHA256ca21843d281eb999a14a62d824f1914444bb34e47ce2517e13acddb5ffc329b4. Final installed overlayde40090d2b274dee1f6b549c3d27781c463742d7. Squared Away 3 Purchase Admission Dev 15.zip10754811bytes SHA25600fe1a58ff382ea302ee998c449d68aa149199ced1b12daa767033ac8d421de1. Earlier same-path overlay4ace package72e6a39... andb13packagef290307... superseded in place, not retained duplicate downloads. Twelve payloads verified. Public engine, production and pristine untouched.

PID33828 first strict verifier failed only PDAcondition10000->9999 while native purchase/count/money checks passed. Preserve purchase-attempt1.log/inv/jpg. Source-derived vanilla item_device.script on_trade_opened (line272) sets device condition to c_full(default0.9999); on_trade_closed(line302) restores temp_con through alife_process_item. It registers ActorMenu_on_trade_started/closed at400/401. Overlayb13cbf6f closes the normal trade dialog before final snapshot/export/save, preserving the exact-record assertion. No production script modified.

PID4380 corrected scripted test PASS:67-item basket refuses before callbacks;65accepted as one batch,65unique client acknowledgments, actual pending-save refusal,71items4928g and original6records exactly unchanged. Actor1000001->926876, seller1002862->1075987, spent73125. Retained19778/19779 at seller19456; full retry refuses unchanged. PID28956 fresh sqa3_purchase reload preserves all71records, exact export61df8854a7d402e773b2cd487abb2d875783d18d9d0cbe8475b097ea068a99c5, both balances and both sources38gcondition1. Evidence ci-40488824434/purchase.log/inv/-verification.json, purchase-reload equivalents and purchase.jpg. All exit0.

PID22576 physical mouse test PASS: drag seller bandage19713 into purchase basket (first positioning drag did not move item; observed retry did), click visible Buy. Exactly1batch/client acknowledgment, actor1000001->998876, seller1001738->1002863,1125RU conserved.7items2496g, all6baseline records unchanged after normal trade close. PID27164 fresh sqa3_purchase_mouse reload preserves exact records/export0386d4b1be8f345cc5bde55bba7a18fb03354e9bd42e7f9830b61797cc918f82 and balances. purchase-mouse.log/inv/-verification.json and purchase-mouse-reload equivalents. PID19388 full sqa3_purchase save: physical drag one of the two remaining seller bandages into basket, click Buy; visible Purchase does not fit. Items and money unchanged; selection retained. Native no_fit, no committed batch/client acquisition, exact71records/export61df..., both retained sources and balances unchanged. purchase-refusal-mouse.log/inv/-verification.json/jpg. All exit0. Physical buy/refusal evidence is distinct from scripted65-item batch evidence.

final-installed-save-audit.json verifies12installedpayloads and original test.dds/scoc/scop against pre-test-save; elevated process check zero games. Superseded exact-hash Dev13ZIP10728865bytes andcb5downloadEXE26809344bytes removed (37538209total); dev13-superseded-download-cleanup.json. Dev14package/EXE working rollback and Dev15current retained, all historical logs/source/saves retained. Previous rejected rollback deletions were not retried. 104core debug/release checks and independent restarts remain the current core evidence. Sales, external equip/belt/use admission, loaded-world subtrees, arbitrary callback mutation and unusual post-server/client failure reconciliation remain OPEN. No complete-mod claim.


### Direct empty-slot native candidate - 2026-10-27

Native95dffc082169eb6979e92a8866de298cb8d83155 CI40494446158 compiling; e4cf831f/CI40494217255 andb646a1a3/CI40494318523 cancelled/superseded, no artifacts downloaded. GroundPickup.prepare accepts optional equipment_slots and uses existing TransferSession Equip command instead of grid find_placement.106debug/release behavioral tests and normal/logical fresh restarts pass in native-direct-equip-build, linked unchanged native-ground-pickup-build libraries. Added tests prove full1x1bag allows2x2incoming into empty equipment bit4, rejects same-slot competition and invalid mask, and preserves owned source on native rejection.

GE_SQA_ACQUIRE now carries trailingu16slot (ffffordinarybag); all sender/receiver changes share this isolated build. UI ToSlot routes external items before cell removal; refuses trade-view free equip and pending actor equipment changes. Server validates live source/identity, native availability, native item-slot compatibility (matching CanSetItemToList; native CanPutInSlot alone does not restrict actor item types), and conflicting outfit/belt displacement. Native accepted model reserves final Equipment; CInventory::Take applies pending requested slot before placement and PickupTaken requires exact final equipped mask. Unusual state changes after server ownership still fail closed by save gate; no universal compensation claim. Direct occupied-slot swaps, belt/use and broader scripted producer parity remain open.

Overlay83b51b03 prepares physical direct-equip fixture from full sqa3_purchase: spawn two crate AK74s, set condition0.83/0.91 and magazine5/9, drag one into empty primary slot3, try other into occupiedslot3. Verifier requires72records with all original71unchanged, exact source retention, actual refusal/clientack/savegate and both weapon mass/condition/ammo unchanged; fresh-reload mode compares exact records and weapon state. Python verifier syntax passed; no runtime acceptance yet. Dev15enginea6036dbe/overlayde40090d remains installed; no games running.


### Dev16 direct equipment runtime finding — 2026-10-27

Native95dffc082169eb6979e92a8866de298cb8d83155 CI40494446158 SUCCESS artifact12730054661; EXE26864128bytes SHA2567954f15f23748f9b0887dc08f760d857447769b0f9e6b2f2fb9186bf72c16fb6. Overlay8e9d646c3142c34bf291ab518b1ff20a408cd055. Squared Away 3 Direct Equip Dev 16.zip10759183bytes SHA256b3bfae8931217c6ad2fd37c20b4429f2216a5ae7232a5d6b4b0133c2c918b176;12payloads verified installed only in lab. PID8760 exited0. Physical source-to-empty-primary drag accepted live20225 into slot3, crate19969 retained20224.72records; original71exact; rifles preserve condition0.83/0.91 and ammo5/9. Native acknowledgment and pending-save refusal logged. Evidence ci-40494446158/direct-equip.log/inv. Strict verifier FAIL: second observed drag did not produce required native occupied-slot refusal. Do not weaken this check or claim refusal acceptance.

After normal menu close active live11664 remained the PM, not acquired20225. Ordinary UI ToSlot sends GEG_PLAYER_ACTIVATE_SLOT, but guarded direct admission returns before it. Native follow-up153dd4f6d12fe400a5d09dbff8a2da2c07a857c4 restores the same event at end of Take only when the exact requested item is actually in its acknowledged slot. Verifier now also requires active==acquired. Isolated build pending; repeat native drag/refusal and reload required. Dev15 remains accepted rollback; installed Dev16 is an unaccepted candidate. Public engine/pristine/production untouched.


Dev16 follow-up evidence: PID14304 owned-box regression exited0 and verify_native_owned.py PASSED65accepted/65acks/65pending-save refusals, two exact retained sources19712/19778 at19456,71items4928g and exportf28fba12de757eabc99a05af8846650f98562c76cbd162435430d3ef0f9bf06f. ci-40494446158/owned-box.log/inv/-verification.json.

PID14504 direct-equip-attempt2 exited0; repeated physical drags still produced no direct-slot refusal record; doubleclick did produce ordinary no_fit on retained20224. PID36388 direct-equip-reload exited0; only logicalrecord175condition changed9100->9098. Other71records exact, both rifles' parents/ammo retained. direct-equip-attempt2.log/inv, direct-equip-reload.log/inv, direct-equip-reload-findings.json preserve the failure. Root cause verified in Weapon.cpp:1070, xrServer_Objects_ALife_Items.cpp:539/557, net_utils.h:193 and NET_utils.cpp:255: q8 writes round(condition*255), reads byte/255.0001. Fixture now sets212/255.0001 and232/255.0001; exact assertions retained. Native codec unchanged.

Current native candidate a34327d90e15e840580914cf46055fc80de73798 CI40498004943 compiling;153dd4f6/CI40497400960 superseded/cancelled. Includes activation fix and lab-only OnItemDrop trace distinguishing delivered event/list/slot from missing input. Overlayf6b60add893e8f1ca27418635f37ccd85d3b2885 adds separate physical direct-unequip fixture/verifier and q8-representable conditions; Python syntax checks pass, Lua loaded in native PID6612 without startup error. Updated same-path Dev16 ZIP10760037bytes SHA25622e704dee1cab995b701a16609217740c69d6cab05e16c34753c18171e865a92,12payloads verified installed. Earlier Dev16ZIPb3bfae... superseded in place. Direct-unequip test still underway.


Dev16 direct-unequip acceptance: engine95dffc08 +overlayf6b60add. PID6612 physical primary-slot3 rifle20225 ->crate19969, retained20224. Both exact condition/ammo/mass preserved; actor returns to71items4928g with original71records exact and export61df8854a7d402e773b2cd487abb2d875783d18d9d0cbe8475b097ea068a99c5. PID13772 fresh sqa3_direct_unequip reload preserves identical actor export and both world weapons. ci-40494446158/direct-unequip.log/inv/-verification.json and direct-unequip-reload equivalents. Both exit0. verify_native_direct_unequip initially incorrectly enabled acquisition-only preserve_initial; corrected to final snapshot agreement plus explicit exactly-one expected rifle removal/all surviving records exact/both native weapons retained. No engine fix was needed for this bounded path. Finite target capacity, rigs and loaded-container subtree transfers remain OPEN. PARITY-COVERAGE X02 updated to bounded native slice, X01 remains unaccepted.


PID33548 context-action test exited0: physical empty-slot drag acquired20225; right-click remaining rifle20224 then visible Move to slot produced native direct-equip refusal slot3 and Equipment slot unavailable status.72items8324g; all original71records exact; condition/ammo pairs(8314,5)/(9098,9), export8ba67651a171a3d5ea0a62673038264a77b613f63a0f7173f4da40e0b0494855. Active remains11664PM; NOT accepted. ci-40494446158/direct-equip-context.log/inv/jpg/-findings.json. Physical occupied-slot drag remains unverified; context action is not substituted as proof of that input. installed-save-audit.json verifies12payloads and originaltest.dds/scoc/scop; elevated process count0. Current fixture syntax checked with LuaJIT2.1 from existing optional-bodycam-addons-20261019/validation-tools; Python verifiers compile. Fixture now captures post-close weapon screenshot and requires exact q8condition/ammo pairs; nativea34327d9/CI40498004943 still compiling.


### Dev17 direct-equipment acceptance and remaining input check — 2026-10-27

Nativea34327d90e15e840580914cf46055fc80de73798 CI40498004943 SUCCESS artifact12733070349; EXE26865152bytes SHA256e82ec416b783d0b55506851a1a5f1041ce2d2522476f65cce39459ed41bd4ff8. Installed overlayb306a3efc0f3bfa566c9a2fe3618e6e4faf83367. Squared Away 3 Direct Equip Dev 17.zip10763631bytes SHA256b76784b893ec15c6d5a855da120a19446960dc616bd31e1f68a0b6e95f376db1;12payloads verified. PID26668 physical empty-primary drag recorded live20225 from7to1 compatible1slot3; acquisition/pending-savegate/clientack pass. Context Move to slot of20224 refuses occupiedslot3, stayscrate19969. Afterclose active20225 proves activationfix.72items8324g, original71exact, condition/ammo(8314,5)/(9098,9), export8ba67651a171a3d5ea0a62673038264a77b613f63a0f7173f4da40e0b0494855. PID39496 fresh sqa3_direct_equip reload preserves exact72records, both native weapon states and active20225. Both exit0; verify_native_direct_equip.py PASSED both. Evidence ci-40498004943/direct-equip.log/inv/-verification.json, direct-equip-reload equivalents, direct-equip.jpg and direct-equip-active.jpg.

Trace contains only successful empty-slot drop; repeated occupied-slot drag attempts (including clear slot edge) did not reach recorded handler. Context refusal is verified separately. Do not claim occupied-slot drag acceptance or diagnose engine failure from missing input. Current next test is loaded-backpack-to-crate refusal UI consistency; loaded world subtree transfer remains unsupported.


### Loaded release refusal preflight and bulk retention candidate — 2026-10-27

Dev17a34327d9 +overlayb602feaa, PID25628 exited0. Physical drag loaded backpack19200 from pockets into emptycrate19456 visibly moved its cell to crate and showed0.8kg there while medkit stayed in carried-backpack panel. Native server refusal retained all6exact records,2458g, export46193cb2fac38b7b733e4864815d4addbbc6dfc2c16c1a8c26de46c96972a3d3 and crateempty; reopening restores correct view. ci-40498004943/loaded-release-before-fix.log/inv/-verification.json plus loaded-release-before-fix.jpg and loaded-release-reopened-before-fix.jpg. State-verifier PASS is explicitly not UI acceptance.

Candidate0be12d07a14152ffab013acaf93a60d2a0f06b76 CI40502480472 compiling. ToDeadBodyBag checks InventoryNextCanRelease before RemoveItem and displays refusal; global move_item_from_to guards actor outgoing event dispatch. InventoryNextCanBulkRelease retains items with nonactor logical parent and loaded containers, so Move all cannot flatten a loaded subtree. Bulk loot paths snapshot eligible live IDs before synchronous callbacks can mutate cell lists, then re-resolve current item/owner before dispatch. Manual child moves still use ordinary release checks. Native protection is opt-in logical storage; public engine/production untouched. New loaded-release and loaded-bulk fixtures/verifiers require exact protected subtree and native source state; bulk expects three loose items moved and bag/medkit/PM retained. Runtime checks pending.

Dev17package overlayb26e8cf27d1173e1f8fdbb9007e9138d97a74117 currently installed,12payloads verified; same-path ZIP10764842bytes SHA2569ee6bb359ce88c3fbad27269267ee6483662c4c73b2b1b8cc870ab5561fd0d2d. Earlierb602ZIP10764499SHA7930ddae... andb306ZIPb76784... superseded in place. Exact-hash/resolved-path cleanup removed Dev14/16 ZIPs and downloaded EXEs,75164740bytes; dev14-dev16-superseded-download-cleanup.json. Dev15accepted rollback and Dev17current retained, allsource/saves/logs preserved. Initial preflight stopped on a transcription typo in Dev14 expected hash; nothing deleted until correct previously recorded hash matched.


### Native-input carried rotation and reload — 2026-10-27

Dev17 engine a34327d90e15e840580914cf46055fc80de73798 with overlay16e8d783c6cc1e37f70db30584714361dc7b00a0. PID13272 native selection/reload-binding fixture accepted rotation of logical103 within parent105 at5,6,1; six items2458g, other five records exact. PID34564 fresh reload preserved all six records exactly. Both exited0. Evidence: ci-40498004943/logical-rotate.log/inv/-verification.json and logical-rotate-reload equivalents. Export hashes1c2a7de289473c486c7246386dade78392a20ad245dc24000483b8316c7226d5 and655eff7561f560f902ec0a2357ec69f12ba51ce22397ee01bd18d50eb10f4273 (item dictionaries exact; export metadata can differ). Native fixture dispatch is explicitly NOT physical keyboard evidence. Boundary fixture pending. Same-path Dev17 ZIP10765800bytes SHAe854eed7741b261b911840067f1136814aa1371521fc02a3e094ecd5023e61cc; all12 lab payloads hash-verified before launch.


### Boundary rotation fixture accepted — 2026-10-27

Dev17/overlay16e8d783: PID17172 native-input edge rotation refused, logical103 stayed parent105 at7,6,1; all6items2458g, other5records exact. PID30544 fresh reload exact, both exited0. ci-40498004943/logical-rotate-edge.log/inv/-verification.json and reload equivalents; both exports4f64afff8173fbaf39cf39dc8bd23b0bb18276c4dc54c167114675cb0ed16b9c. This is native fixture input, not physical R-key acceptance. Dev18 loaded-storage safety build0be12d07 CI40502480472 succeeded, artifact12734277570, EXE26867200bytes SHA3633396cd254c3636868cf60c427fd954b0c6dc21f69e097d82659e01bb6bbb5. Packaged with overlay16e8d783: ZIP10766857bytes SHA4c4472041578e1d2448648536b22753f2382034ee1d6b2e429e7f3fc7818e5ca,12payloads verified and installed only in isolated lab. Runtime loaded-container checks ongoing.


### Dev18 loaded release refusal accepted — 2026-10-27

Engine0be12d07/CI40502480472 +overlay16e8d783, PID12776: physical carried bag19200 tocrate19456 refused before UI removal; visible pockets bag/medkit grid intact, crateempty, message Cannot transfer this item. Inventory unchanged. Native state exact6items2458g, export46193cb2fac38b7b733e4864815d4addbbc6dfc2c16c1a8c26de46c96972a3d3. PID31464 fresh reload exact, both exited0. ci-40502480472/loaded-release.log/inv/-verification.json and loaded-release-reload equivalents; loaded-release.jpg captured13:39:01. Visual inspection and strict state verifier both passed; loaded subtree world transfer remains unsupported. Bulk test ongoing.


### Dev18 loaded bulk retention accepted — 2026-10-27

Engine0be12d07 +overlay16e8d783: PID32468 physical Move all tocrate19456 transferred original PDA100/bandage101/bread104 with exact live IDs, mass and condition; retained logical102medkit parent105,103PM equipment,105backpack. Actor3items2130g, exact protected records, exportd5ebe13bc54f563c5cb634fc46e906284d32ffa67ba8447f2b9086518255ab51. Visible UI showed loose items in crate and backpack+medkit intact with refusal feedback. PID27724 fresh sqa3_loaded_bulk reload exact on both actor/source; both exited0. ci-40502480472/loaded-bulk.log/inv/-verification.json and reload equivalents; loaded-bulk.jpg13:42:04. installed-save-audit.json confirms12payload hashes and originaltest.dds/scoc/scop unchanged. Loaded subtree transfer is refused, not implemented; corpse bulk and larger content coverage remain open. Direct-unequip guard regression underway.


### Dev18 direct-unequip regression accepted and checkpoint — 2026-10-27

Engine0be12d07/overlay16e8d783: PID11680 physical primary rifle20225 ->crate19969; retained rifle20224; original71actor records4928g exact, both weapons retain condition/ammo/mass. PID10412 fresh reload exact; both exited0. ci-40502480472/direct-unequip.log/inv/-verification.json and direct-unequip-reload equivalents; both exports61df8854a7d402e773b2cd487abb2d875783d18d9d0cbe8475b097ea068a99c5. Elevated process audit found zero Anomaly/xrEngine/VerifiedDX instances. Existing12payload hash audit and originaltest.dds/scoc/scop comparison pass. No production/pristine/public/FG changes. Read installed lab runtime files, logs and saves only; package atlas remains unmodified Squared Away Hotfix1 FieldKit source.

Exact-path/hash cleanup removed Dev15 ZIP10754811bytes SHA00fe1a58ff382ea302ee998c449d68aa149199ced1b12daa767033ac8d421de1 and CI40488824434 downloaded EXE26860544bytes SHAca21843d281eb999a14a62d824f1914444bb34e47ce2517e13acddb5ffc329b4:37615355bytes freed. Manifest dev15-superseded-download-cleanup.json. Dev17 rollback, Dev18 current and all source/evidence/saves retained. Physical R-key and occupied-slot drag automation remain unverified; native rotation and context refusal are separately verified, not substitutes. Full77-row parity remains open.


### Physical-input retest limit - 2026-10-27

Dev26 enginefd20bc7b/overlayd20fb6e (stash mode disabled): PID38548 physically selected pistol103, then Computer Use sent R. No native DIK19 or rotation event appeared; final103 remained parent105 at5,6,0. Clean exit0/6items2458g does not establish rotation. Evidence ci-40549072785/physical-rotation-verification.json has status manual_input_required. Do not repeat unchanged injection attempts.

PID5252 physical first crate rifle -> empty primary slot succeeded, acquired20225. The remaining20224 stayed in crate19969 after an occupied-slot drag attempt, but no native drop callback for20224 appeared. This is not accepted native refusal coverage. Exact72records/8324g, original71records unchanged; both magazines/condition and active weapon preserved. PID27300 fresh reload passed exactactor/weapon comparison; both exit0/no timeout. Evidence occupied-slot-drag-verification.json is partial; occupied-slot-drag-reload-verification.json proves persistence only. Existing context-menu refusal and scripted rotation evidence remain separate.
