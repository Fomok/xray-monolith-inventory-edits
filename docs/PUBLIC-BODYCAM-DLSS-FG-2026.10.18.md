# Squared Away + Bodycam + DLSS + Frame Generation

Optional DX11-AVX engine package, version `bodycam-dlss-fg-2026.10.18`.
Keep Squared Away installed. This is an additional experimental option; the
standard, Bodycam-only and Bodycam+DLSS options remain available.

This release combines the unchanged Test 8 automatic-start engine with the
patched NR 0.8.91 companion. Fomok confirms the combination works in game.
That confirmation does not establish every scope, visual setting or long session.
No FPS gain is guaranteed; neural rendering adds GPU work and can reduce rendered FPS.

## Requirements

- Windows 10 version 2004 or newer, or Windows 11; 64-bit OS and AVX-capable CPU.
- NVIDIA RTX 40/50-series for this native 2x FG path, with a compatible current
  NVIDIA driver. Locally tested on RTX 5080 / driver 617.14. SDK capability checks
  decide availability; this is not an RTX 20/30 FG unlock or AMD FG package.
- Enable Hardware-accelerated GPU scheduling in Windows Settings > System >
  Display > Graphics > default graphics settings, then restart Windows.
  See [NVIDIA requirements](https://github.com/NVIDIA-RTX/Streamline/blob/main/docs/ProgrammingGuideDLSS_G.md#10-requirements).
- Use DX11-AVX, SDR/windowed or compatible borderless presentation, MSAA off,
  and active temporal upscaling. Exclusive fullscreen/unsupported formats fall
  back to native presentation. DLSS/DLAA is the tested NR configuration.
- GAMMA shader setup: SSS 23.5, your actual Atmospherics version, and the two
  compatibility patches linked below. Select add-ons only when installed.

NR is a separate experimental feature of the supplied OptiScaler fork, using the
included 310.8 compatibility model. Its availability depends on the driver/runtime.
The model is modified, not an unmodified NVIDIA-signed DLL. The engine's six
Streamline/DLSS-G runtime DLLs retain their verified NVIDIA signatures.

## Installation

1. Close the game and MO2. Back up the existing `bin`, `db/mods`, OptiScaler INI
   and enabled shader/UI patch setup. Preserve a working complete engine package.
2. Extract **Squared.Away.Engine.Bodycam.DLSS.FG.DX11.AVX.zip** into the Anomaly
   game folder, merging its `bin` and `db` folders together. Do not install this
   runtime ZIP as an MO2 mod. It includes the tested engine, patched companion,
   clean configuration, SR/NR models and Streamline; no earlier test package is needed.
3. Keep one OptiScaler proxy only: the supplied `bin/dxgi.dll`. Back up and move
   out an older OptiScaler proxy under another name; do not remove unrelated
   ReShade/mod loaders blindly. Back up and move out obsolete
   `bin/nvngx.dll_dlssnr.dll`. **Keep `bin/nvngx_dlssnr.dll`**, the actual NR model.
   Replace the supplied INI or carefully merge personal tuning while preserving
   `[FrameGen] External=true`, `Enabled=false`, and `[Menu] OverlayMenu=false`.
4. Install [Bodycam GAMMA Patch 1.1.0](https://github.com/Fomok/xray-monolith-inventory-edits/releases/download/bodycam-dlss-2026.10.18/Bodycam.GAMMA.Patch.1.1.0.zip)
   and [Bodycam DLSS Grass Motion Shader Patch](https://github.com/Fomok/xray-monolith-inventory-edits/releases/download/bodycam-dlss-2026.10.18/Bodycam.DLSS.Grass.Motion.Shader.Patch.zip)
   through MO2. In the shader installer select SSS 23.5 and your installed
   Atmospherics version (or None); choose only installed optional add-ons.
   Put the shader patch below Bodycam, SSS and Atmospherics so it wins conflicts.
   Disable overlapping older DLSS/FSR compatibility patches.
5. Install **Squared.Away.FG.UI.Scope.Patch.zip** through MO2, below those patches
   and any older personal FG UI/shader mod. It must win these exact loose files:
   `gamedata/scripts/modxml_dlss_options.script`,
   `gamedata/configs/text/eng/st_monolith_dlss.xml`, and
   `gamedata/shaders/r3/scope_reference_image.ps`.
   Verify their winning provider in MO2's Data/conflict view. Loose files override
   packed DB files; the grass patch contains an older UI script/XML, so the final
   FG patch is required even though matching copies are packed in the runtime.
6. If upgrading old personal tests, back up and remove their obsolete
   `db/mods/zzzz_personal_fg_test7.db0` (or other specifically identified older FG
   override). This release replaces `zzzz_personal_fg_test8.db0`; do not leave
   duplicate renamed copies or delete unrelated archives. Clear the shader cache
   through the launcher after installing/changing shader patches.
7. Launch your ordinary shortcut > MO2 > Play using DX11-AVX. No special launcher,
   environment variable or command-line opt-in is required.

## In game

In Video > Basic, turn MSAA off, select DLSS/DLAA and its quality mode, and Apply.
Set **NVIDIA frame generation** to **On (2x)** and Apply. **Frame generation inside
PiP scopes** separately enables interpolation for supported Bodycam Reference
optics. PiP Off suspends FG while scoped; unsupported legacy scopes, NV/thermal,
loading, menus and unsafe UI can also suspend it. Cancel preserves saved choices.

Press Insert for OptiScaler/NR controls; enable its FPS Overlay if desired. Keep
OptiScaler's own FG disabled and its presentation overlay disabled: the engine
owns FG. Rendered/Presented counters report actual runtime counts, not a fixed
multiplier or physical monitor scan-out. The frame limit targets presented output:
100 with active FG means roughly 50 rendered plus 50 generated; off means 100 rendered.
The provided INI starts with pre-SR NR on, one pass, and leaves new finished-picture,
compression and debug modes off. NR can be disabled separately if its cost is unwanted.

If FG says unavailable, check GPU/driver/HAGS, presentation mode, MSAA and shader
overrides, then fully restart. Unsupported startup falls back to native DX11.
The existing `-dxgi_old` switch is an optional recovery route, not required normally.

## Rollback

Close the game. Restore the backed-up complete engine bin/db and companion INI,
including the old helper if that prior companion needed it. Disable the new FG
UI/scope patch and restore the previous shader/UI priority. Remove this release's
FG override DB if the restored package does not contain it; clear shader cache
after reverting shaders. Do not mix an old executable with new packed/UI files.

## Source, notices and validation

- Engine binary: [fa1b97c3](https://github.com/Fomok/xray-monolith-inventory-edits/tree/fa1b97c3c85bd2bfb1704dc937e2fe7ce4d09169),
  [build 39147479433](https://github.com/Fomok/xray-monolith-inventory-edits/actions/runs/39147479433).
- Companion: [personal patch dadc010c](https://github.com/Fomok/xray-monolith-inventory-edits/tree/dadc010c8de129dfa28efd8f2fa11b64271029a1/tools/optiscaler)
  on [NR fork f45ccf3a / release 0.8.91](https://github.com/wilsjo2/OptiScaler-DLSSNR-PreSR-Multipass/tree/f45ccf3a761df91450959d325ac0166cba5364c9),
  [build 39151582217](https://github.com/Fomok/xray-monolith-inventory-edits/actions/runs/39151582217).
- This release supplies a separate **OptiScaler.NR.0.8.91.Corresponding.Source.zip**
  containing the complete patched GPL source and pinned submodules, and
  **LICENSES-AND-NOTICES.txt** with redistribution notices. These are compliance
  materials, not the private engine developer merge kit. The developer merge ZIP
  is not published. Runtime files and their source/model provenance are listed in
  **runtime-manifest.json**. Keep the license notices with redistributed copies.

Automatic-start/fallback, GPU inputs, telemetry and settings checks passed for
the engine. The final companion passed NR diagnostics, eight cleanup cycles and
a foreground 1,800-frame NR/FG/menu run (1,360 two-present reports; 3,160 total SDK
presentations). Initial shutdown crashes were fixed before delivery; focus-lost
tests are preserved as failures, not generation passes. Public packaging only
changes two UI descriptions to remove obsolete wording; the script and scope
shader behavior and both tested binaries are unchanged. Packed UI CRC/payloads,
loose overrides, binary hashes and archive contents are verified before publishing.
