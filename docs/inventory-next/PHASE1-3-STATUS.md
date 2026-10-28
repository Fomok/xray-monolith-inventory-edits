# SQA3 Phases 1 through 3 active implementation checkpoint

28 October 2026. Authorized through end of Phase3, then stop/report. This checkpoint is not completion of those stages.

## Scope

Backpacks have authored compartments and masks. Only backpacks and rigs grow externally. Default configurable policy prohibits storage nesting into portable-container interiors, while actor pockets/root, world roots, equipment and pouch mounts remain permitted. Existing nested state must remain recoverable. Preserve original rig geometry and center the whole effective extent after pouch changes. Custom real equipment slots must have a separate native registration domain outside every vanilla reserved/unused ID; no native pistol slot change. Final launcher opens the main menu and must permit New Game/Load without an autorun or forced save.

## Confirmed progress

Source contracts: PHASE-2-CATALOG-CONTRACT.md. Latest audited source handoff is C:/GAMMA/sqa-previews/inventory-next-20261028/catalog-handoff/catalog.json,49 records, corrected tall-first masks. Do not use initial41-record masks. HANDOFF.md dependency follow-up supersedes its earlier paragraphs. No new installed-dependency audit duplicated; Mod/UI records its18 inputs.

Native equipped-backpack stash creation PID33788 and fresh reload PID38732 pass. Original loaded subtree identities and state preserved; native_place changes3→1 after actual equip, handled only by explicit --equipped verifier mode. Test fixture asserts actual native slot13 before use.

Dialog cancellation, proxy timeout, partial release rollback, full-pocket refusal and removal callback cancellation pass, each with exact fresh-process reload. Evidence C:/GAMMA/sqa-previews/inventory-next-20261028/phase1-3/*-verification.json and run JSON/log/inv/world files. Fixtures call native dialog/Take All routes, not physical OS clicks. First timeout checked too early; fixed with6s wait for5s timeout. First nofit spawned filler into recovery; fixed by explicitly placing six bandages into the six vacated legal pocket cells before retrieval. Old failed logs retained.

Core131 behavioral tests pass in debug and optimized builds, with independent ordinary/logical/world restarts: phase1-3/core-policy-v4/. New checks cover compartment import, malformed-mask atomicity, configurable new-nesting refusal, retained old nesting, native fact rebuild under newly prohibitive policy, and catalog application preserving instance condition/mass/upgrade data.

Importer tools/inventory_content/import_sqa3_catalog.py compiles49 definitions into phase1-3/catalog-native.ltx/json. All22 authored rig masks compare exactly; non-backpack/non-rig exteriors remain fixed. Nylon and its alias have no amp_layout: the source count/skyline fallback is reproduced, not invented authored coordinates. Growth stages are explicitly provisional configurable test balancing. Pouch grants and box slot rectangles are resolved into compartment IDs. Offline import is not native catalog acceptance.

## Current native build

a088223456f7730c911e74f60f7f571d866a5d44 pushed; CI40634592708 in progress when recorded. Adds native definition reader/application, pure-ledger footprint projection, safe reapplication of prior nested placements and startup respecting g_always_active. No actual catalog item/config/asset package installed yet. Build success and runtime acceptance pending.

Main-menu launcher tools/inventory_lab/Start-Squared-Away-3.ps1 has no fixture/forced save, verifies lab manifest and refuses a second instance. Packager includes it when present at the pinned overlay commit. Syntax passes; final launcher/gameplay testing remains pending.

The lab currently has engine06db7402 plus test overlay206629f2, not the a088 engine. Final Dev32 ZIP remains untouched; phase1-3/stash-regression-overlay.zip is intermediate fixture material. Do not label it a Phase3-ready mod.

## Active issues and next work

1. Named-stash travel PID33168 timed out, watchdog closed it. It did not execute the actor fixture while unfocused. Native source shows unconditional startup Pause despite always-active; a088 corrects the guard. Runner has an uncommitted -Background switch using a separate appdata/sqa3-fixture-user.ltx with g_always_active on and hidden process launch. Original user.ltx remains unchanged. Test background behavior after a088 succeeds; do not infer hidden visual acceptance from this mode. The failed travel is not PASS.
2. Finish travel/return/reload and remaining Phase1 source switching/disappearance/window/scroll checks. Actual physical R-key/occupied-slot drag semantics remain manual/unverified; record separately and continue independent work.
3. Complete Phase2 native catalog item sections, meshes/icons/text dependencies, category acceptance, stateful stack/picker and authoring validation. Existing importer alone does not provide native item sections/assets. Use fresh II_ATTCH CInventoryItemObject storage items or a dedicated new class; the released II_CONTR class is absent in this clean engine and must not be copied blindly. Source shared meshes are meshes/squared_away/rig_shared.ogf and box_shared.ogf with matching textures, plus ui_amp_* atlases. Do not ship legacy inventory scripts.
4. Phase3 custom slot registry is not implemented. All vanilla SInvItemPlace slot fields are6bits and native APIs useu16; reserve the entire existing domain, including unused slots. A registry alone is insufficient: connect real CInventory custody/placement, events, save/reload, action access and UI. Preserve the visual-only pistol frame. Complete actual rigs/mounts/pouch capacity, centering, condition recovery, removal/refusal, upgrades/dismantle/quick-use/ammo/equipment routes and native tests before claiming Phase3 done.
5. Package/install/test final build in lab, leave game closed, document how to get representative items and start a new game. Planning owns the Desktop shortcut last. Stop before Phase4.

Keep Sol Medium, bounded reads, batched relevant checks; no subagents/model escalation initiated. Public/personal FG/baseline/production remain untouched. Original test save must be rehashed at final audit. Preserve other chats' untracked HIDDEN-NATIVE-LAB.md and NESTED-ACTION-COMPATIBILITY.md.
