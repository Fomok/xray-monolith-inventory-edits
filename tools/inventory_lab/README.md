# Inventory Next: native development first slice

Status (27 October 2026): pristine upstream baseline pushed and verified. This derived
branch adds a small Lua development helper and this feasibility record. **No standalone
inventory application exists yet.** The helper has offline guard tests, not an in-game
rendering pass. It neither reimplements inventory nor changes item behavior.

## Pristine baseline

Latest published release at verification: [upstream 2026.10.11](https://github.com/themrdemonized/xray-monolith/releases/tag/2026.10.11),
published 11 October 2026 at 09:21:06 UTC (11:21 Warsaw). Both GitHub's latest-release
endpoint and the recent release list were checked. We fetched the release tag, not
`all-in-one-vs2022-wpo` HEAD and not the separate MT development branch.

- Commit: `c57fe5b9fab4fe0506659db03137c52d2500eb2f`
- Tree: `3d1baeed6aeb66d3a0d60563f81938601013b9f2`
- [Pristine branch](https://github.com/Fomok/xray-monolith-inventory-edits/tree/inventory-next-upstream-baseline)
- Baseline and upstream resolve to the SAME commit; no extra documentation commit,
  cherry-pick or SQA/Bodycam/DLSS/FG/pivot merge. GitHub's commit API independently
  returned the same commit and tree after pushing. Push required a nonexistent
  remote ref, so it could not overwrite another branch.
- `baseline.json` pins this evidence. Future newer releases do not silently move it.

All development here belongs on `inventory-next-harness`, derived from that commit.
Do not commit these files to the pristine branch. The existing public engine branches,
releases and installed games are unchanged.

## Why the actual inventory is not a standalone widget

References below are relative to the pinned repository root, not the modified SQA engine.

| Dependency | Source evidence | Consequence |
|---|---|---|
| Live level and actor | `src/xrGame/UIGameCustom.cpp:161` (`ShowActorMenu`), especially `Level().CurrentViewEntity()` and actor verification | Cannot open the genuine actor menu in an empty desktop window by simply constructing it. |
| Game UI and state | `src/xrGame/ui/UIActorMenu.cpp:44`, `:198`, `:208`; character information, indicators, zone map | UI needs real owner/world services or a deliberately extracted interface. |
| XML, textures, localization, sound | `src/xrGame/ui/UIActorMenuInitialize.cpp:47`; `actor_menu.xml`, upgrade/character/item UI; `UICellItem.cpp:55` | Repo is an engine overlay, not all Anomaly game data. A licensed/owned compatible base installation is required. |
| Inventory model | `src/xrGame/Inventory.cpp:5-31`, constructor `:71`; actor, weapon, settings, level, AI, HUD | A fake list of item rectangles would not exercise slots, ownership, capacity or real item lifecycle. |
| Transfers and equip | `src/xrGame/ui/UIActorMenuInventory.cpp:59` onward | Uses `NET_Packet`, real parent/object IDs and game event delivery even in single player. |
| Grid rendering/input | `src/xrGame/ui/UIDragDropListEx.cpp:226`, `:955-1051`; `UICellItem.cpp:352` | Uses Level input, UI font/scissor/render services and Device frame/render callbacks. Not a portable renderer-independent grid. |
| Build | `src/engine-vs2022.sln`, `src/xrGame/xrGame.vcxproj` | VS2022 v143 + Windows SDK, precompiled headers, xrCore/xrPhysics/xrNetServer/xrXMLParser/xrCDB/xrAPI and third-party dependencies. No small supported inventory executable target. Local compiler/toolchain was not found. |
| Anomaly/GAMMA scripted frontend | Actual installed GAMMA UI `ui_inventory.script` uses `db.actor`, `level.object_by_id`, sounds and MCM/other scripts | Native `CUIActorMenu` parity is not automatically parity with the later Lua frontend or Squared Away's grid. Nothing from GAMMA was copied to this baseline. |

Thus a complete, faithful standalone application is an extraction project, not a safe
small wrapper in this turn. The lack of a local compiler is an additional validation
constraint, not the sole reason. Existing upstream CI builds the whole game; this
branch does not trigger it, add a build workflow or publish a release.

## Useful first slice: inspect and rebuild the real native UI in one session

`gamedata/scripts/inventory_lab.script` calls upstream bindings directly. It has no
`on_game_start`, scheduler subscription, automatic fixture spawns, item transfer,
save write, config rewrite or replacement UI. The snapshot reads real actor objects.
Reload routes through the actual engine's `level.reload_ui_xml()`.

For an isolated vanilla Anomaly 1.5.3 test installation with the matching upstream
release engine and data overlay (not the production GAMMA installation):

1. Use a separate development game directory and separate appdata/save/profile.
   Do not point the test at production savedgames. Start with a disposable new game.
   The engine repository alone is not a complete runnable Anomaly installation.
2. Put this helper in that test directory's `gamedata/scripts`. Launch its executable
   directly with `-dbg`, without the GAMMA MO2 profile. No modified engine build is
   required for this first slice; the upstream release already exposes the calls.
3. Verify the test actually uses native `CUIActorMenu`: upstream calls
   `actor_menu_inventory.CUIActorMenu_OnMode_Inventory` first, and a true return
   replaces it with a scripted frontend. Even vanilla Anomaly data may supply that
   hook. This helper does not disable it or rewrite the original scripts. If it
   selects a Lua inventory, use only snapshot until a dedicated native fixture is
   prepared; rebuilding the hidden native menu is not a frontend test.
4. Load the test save once. In the console run `run_script inventory_lab`.
   This loads/reloads only this helper and logs sorted real object IDs/sections.
5. Edit an existing `actor_menu.xml` in the isolated development overlay. Close
   inventory, loot/trade and PDA windows, then run:
   `run_string inventory_lab.reload_native_ui("isolated-native-session")`
6. Close the console and reopen native inventory. Compare layout and run
   `run_string inventory_lab.snapshot()` if needed. Do not retain native widget
   references across rebuilding; the engine deletes and recreates them.

**Do not run the rebuild against GAMMA/SQA's scripted inventory and assume it is
safe or equivalent.** The explicit argument acknowledges the isolated native test
session; it does not detect or certify the installation. The helper checks a live
level/actor, required API bindings and closed inventory/PDA, but cannot certify
arbitrary third-party scripts. It is an experimental development helper, not a
player-facing fix. We did not install or launch it in the user's game.

### What reload actually covers

`src/xrGame/level_script.cpp:2207` forwards to
`CHUDManager::OnScreenResolutionChanged` (`src/xrGame/HUDManager.cpp:437`). This hides
dialogs, deletes scope/spot XML caches, unloads the game UI, reloads map spots and
loads/connects game UI again. `CUIGameCustom::UnLoad/Load` recreates native ActorMenu,
PDA and HUD. It is broader than inventory-only reload.

It does **not** reload arbitrary Lua modules, item definitions, textures or all XML.
`CUICellItem::init()` caches `actor_menu_item.xml` in a static `CUIXml` with a static
ready flag, so changing that file still needs restart until a safe explicit cache
reset is implemented. `run_script inventory_lab` reloads this helper using upstream
`CCC_Script`/`CScriptThread` (`console_commands.cpp:1821`, `script_thread.cpp:52`), not
all scripts. In release builds console script commands require `-dbg`.

## Validation and limits

Seven LuaJIT2.1 tests pass: no automatic action, deterministic read-only snapshot,
explicit session acknowledgement, missing/dead actor/level, open inventory/PDA,
missing bindings, and one rebuild dispatch for a closed valid test session.
These tests stub the engine boundary; they do not claim rendered UI, actor transfers,
resource lifetime or gameplay correctness. Source binding/ownership paths were checked
against the pinned release. No C++ changes, compiled app, live scene, packaged fixtures
or measured launch-speed benefit are claimed.

Run offline tests with Python plus `lupa==2.8`:
`python tools/inventory_lab/test_helper.py`

## Next practical stages and acceptance gates

1. **Isolated real-engine session**: supply/verify clean Anomaly data and upstream
   runtime, create a tiny disposable test save (actor, representative real items,
   two boxes; no GAMMA). Confirm native-vs-scripted frontend choice explicitly.
   Gate: open, drag, equip, transfer, save/reload with actual objects and zero errors.
2. **Reliable resource reload**: run the helper's native layout test; add an explicit
   developer-only cache reset and a UI-owned lifetime boundary, rather than calling
   general script reload on live object references. Gate: repeated close/rebuild/open,
   no dangling drag item or duplicated callbacks, edited layout visibly applied.
3. **Engine-backed lab entry point**: add `-inventory-lab` bootstrap on this derived
   branch using normal initialization and a fixed tiny scene/fixture; suppress unrelated
   simulation only where dependencies are proven. Keep real inventory model, event
   handling and renderer. Gate: same fixtures and transfer/save results as normal game.
   This still ships with game-data prerequisites and is not a tiny standalone binary.
4. **Standalone extraction only if worthwhile**: define owner/item/event/render service
   boundaries, build a shared inventory target used by both game and lab, then prove
   identical behavior. Do not duplicate algorithms in a browser to simulate parity.

## Future redesign requirements (record only; not implemented)

- Soft containers change visible footprint with fullness. Decide packed vs equipped
  representation, deterministic resizing, capacity and overflow before implementing.
- Rigid restricted boxes retain fixed exterior size with explicit accepted-item rules.
- Multiple compatible equipped items per category, arrows choose which is displayed;
  selection must not silently equip/unequip or drop hidden items.
- Independent scrollable container grids and nearby-container list; preserve actual
  ownership, distance checks, transfer atomicity and saving, not only visual layout.
- Fixtures must cover resize collisions, full containers, nested-cycle rejection,
  equipment compatibility, selection persistence, transfer failures, nearby object
  disappearance and save/load. These are planned requirements, not delivered features.

## Native core implementation started — 27 October 2026

The initial Lua-helper status above is retained as history. A separate renderer-
independent C++ model now exists under src/inventory_next, with a native test driver
and [current contracts/results](native/README.md). It has compiled successfully:
21 behavioral checks in debug and optimized builds, plus independent-process save
reload. This is not yet linked into xrGame, and no GUI or redesign integration is
claimed. See docs/inventory-next/IMPLEMENTATION-PLAN.md for boundaries and next gates.
