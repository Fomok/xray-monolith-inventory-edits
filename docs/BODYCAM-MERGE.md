# Squared Away + Bodycam compatibility branch

This branch merges Bodycam's 2026.8.4 release source into the Squared Away 2.0.2 engine. It is an experimental compatibility build, not the public Squared Away release.

## Compared revisions

- Squared Away parent: `6312bc321b544cc26980b95a3977d4de965dd770` (`amp-hooks-mt`).
- Bodycam release: `2c4420b60e6abec4685d352615068248200ea485`, [2026.8.4-bodycam-mt](https://github.com/asuparabekon/xray-monolith-bodycam/releases/tag/2026.8.4-bodycam-mt).
- Shared original MT ancestor: `dded87341f0a588c067ecaa04c89261e562527f0` (2026-07-06), also an ancestor of themrdemonized's `all-in-one-vs2022-wpo-mt`.
- Bodycam later copied upstream and PiP updates into linear commits, including `aca9be42` on August 3. The shared ancestor is therefore the Git merge base, not a claim that Bodycam contains only July 6 upstream code.

The release-to-ancestor comparison changes 329 files (41,202 additions / 3,987 deletions). Its camera and shooting changes are integrated with PiP scope rendering, optics scripts and DX11 shaders. Those connected changes are included together; reverse engineering the executable was unnecessary because this tag contains source.

## Merge decisions

Preserved the current Squared Away placement, ownership, real equipment slots, save serialization, direct transfers and UI clipping implementations. Kept newer upstream tab handling, recursive UI locking, renderer event markers, HUD bloom and console compatibility fixes where the older Bodycam copy conflicted. Registered both Shader Bus and Bodycam scripting APIs and both sets of project sources. Combined renderer timing hooks with existing debug markers.

Retained the fork's existing build/release workflows. A separate `AMP-Bodycam-Build` workflow builds DX11-AVX and packs matching gamedata into one artifact containing `bin/AnomalyDX11AVX.exe`, its PDB and `db/mods/00_modded_exes_gamedata.db0`. It does not publish a release. Fixed an inherited malformed renderer project filter and removed a reference to a nonexistent Bodycam source from the unused alternate project. Made inventory tests resolve this checkout rather than a sibling directory named engine.

## Installation for testing

Use the combined artifact's executable and matching DB0 together with Squared Away 2.0.2. Do not mix the old Bodycam EXE or DB0 with this build. Keep backups of the currently working files. This branch does not add a new inventory save format beyond Squared Away 2.0.2; test on a copied save.

The Bodycam author identifies DX11-AVX MT as the tested release configuration. Other renderer configurations are not validated by this compatibility build. For PiP, follow the original [release instructions](BODYCAM-RELEASE.md), including matching external 3DSS PiP compatibility content. That external patch is not invented or bundled by this merge. `r__svpscope 0` selects the original non-PiP rendering path.

## Validation

Passed locally on this merged checkout:

- Native transfer dispatch/completion, asynchronous gaps, duplicate/conflicting requests, destruction, ID reuse and repeated handovers.
- UI reparenting, nested clipping, hidden parents, polygon compatibility and balanced scissor state.
- 324 layout serialization round trips plus malformed packets; native box, rig membership and pouch serialization checks.
- Container transfer placement, world/NPC placement and 10 inventory ownership/access scenarios.
- 3,600 frames of the unmodified Bodycam simulation with movement, ADS and firing, plus PiP entry/exit/reset/fallback checks.
- Lua 5.1 syntax for 19 changed scripts and XML parsing of project/filter files.

Full MSVC engine compilation and gameplay are separate checks. Local tests do not establish rendered scope alignment, input feel, visual quality or game stability.

After the full build succeeds, test walking/sprinting/crouching, aiming and firing with and without scopes, weapon switching/reloading, Bodycam settings, and (if installed) PiP. Then test direct item transfers between equipment/rigs/boxes/stashes/corpses, full-inventory transfers, save/reload and level transition with occupied rigs and boxes. Retain the log if any check fails.
