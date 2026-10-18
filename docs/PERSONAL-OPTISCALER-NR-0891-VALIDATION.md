# OptiScaler NR 0.8.91 personal companion

Live GitHub API checks on 18 October 2026 identified:

- Official release: [OptiScaler 0.9.4](https://github.com/optiscaler/OptiScaler/releases/tag/v0.9.4), published 18 July 2026. Its complete source tree at `7534ad00bf9e590eedb99e8dd9fd8c89dae3654f` has no DLSS-NR implementation, and Config.h has no DlssNr settings. It is not a feature-preserving replacement for this setup.
- NR fork: [0.8.91 prerelease](https://github.com/wilsjo2/OptiScaler-DLSSNR-PreSR-Multipass/releases/tag/v0.8.91), published 23 September 2026, source `f45ccf3a761df91450959d325ac0166cba5364c9`. Standard package selected; optional RTX40-MFG unlock build not selected. The personal engine owns FG.
- Prior working companion: NR fork 0.7.6, source `5019c979656225ec87e3331658f8a1f3ea478244`, with personal menu and shared telemetry modifications.

The [standard upstream download](https://github.com/wilsjo2/OptiScaler-DLSSNR-PreSR-Multipass/releases/download/v0.8.91/OptiScaler-NR-v0.8.91.zip) was downloaded into this isolated development folder. Its SHA-256 matches the release checksum file: `19A2852BB3F88E09075E3CCC66E0318C52E83A5901D9020A10384AE49D59FF77`. All 62 listed payload hashes were verified.

## Why a stock replacement is unsuitable

The new fork removed its old external-FG ownership setting and does not include our measured rendered/presented counters. The personal patch restores startup-only `FrameGen.External=true`, leaves the engine's Streamline modules, Reflex, flip metering and capability queries alone, disables the companion's FG routes for that session, and keeps these choices across INI saves. The menu directs FG/PiP changes to the game's Video settings.

The old D3D11-to-D3D12 untyped resource getter workaround is now covered by upstream `ShaderPipeline_Dx12.h`; the port uses that implementation and its restoration scope. The legacy menu's no-draw resource-state restoration and same-process 64-byte telemetry reader remain patched. No engine rendering code, PiP mapping, shaders or Test 8 executable changes are required.

Initial integration reached the end of rendering but crashed in the driver through the companion's NVSDK_NGX_D3D11_Shutdown1. The same crash reproduced with NR disabled. A captured debugger stack identifies that call; it does not prove a private driver implementation cause. The standard D3D11 shutdown entry completed cleanup and the subsequent D3D12 NR drain in a diagnostic experiment. The source patch selects that standard cleanup only in external-FG mode, retaining the normal native route otherwise. No exception suppression or skipped cleanup is used. The diagnostic binary was restored and is never delivered.

## Configuration and upgrade

The new INI is based on 0.8.91 defaults with compatible explicit choices carried from the supplied Test 8 configuration. It uses `dlss_12`, pre-SR NR, one pass, working scale 1.0, native engine FG ownership and the legacy menu. Finished-picture NR, deferred application, RR residual transfer, spatial compression, debug view and frame hold are off. Removed precision/old capture/unlock settings are not carried blindly. `config-migration.json` records the mapping. New upstream NR code may change visual output; image equivalence is not assumed.

Close the game and back up the complete working Test 8 bin/config. Replace the loaded `bin/dxgi.dll` and INI using the companion package, retaining Test 8's engine, Streamline, SR and `nvngx_dlssnr.dll`. The supplied clean INI is a baseline; preserve any intentional personal tuning when merging. Do not add a second OptiScaler proxy under another name.

Per upstream 0.8.91 instructions, move the obsolete `bin/nvngx.dll_dlssnr.dll` out of the test bin after backing it up. NR now runs inside OptiScaler through the driver; retain the distinct model DLL `nvngx_dlssnr.dll`. The new isolated test omits the obsolete helper. No installed files were changed by this task.

Launch normally through MO2. Keep Video FG/PiP settings as desired. Insert uses the legacy settings menu; the FPS overlay is optional. Restore the previous DLL, INI and helper from the backup to roll back. The complete Test 8 packages remain unchanged.

## Validation

Build [39151582217](https://github.com/Fomok/xray-monolith-inventory-edits/actions/runs/39151582217) passed, including the upstream NR diagnostics regression. Personal patch source is `dadc010c8de129dfa28efd8f2fa11b64271029a1`, applied to the pinned fork source above. Both CI artifact digests and the built DLL hash were verified. DLL SHA-256: `C0EB533DDB19516B5F0A932BDA78D7D8B6B05AC6BEA64B4E33AC599A75E69D0A`.

Final source-built DLL checks:

- Automatic startup with both legacy launch variables absent passed; FG initially off.
- Shared telemetry missing/stale/invalid/variable-count and mode-change self-test passed. Header content is unchanged from the working companion apart from line endings.
- Eight NR initialization/render/cleanup cycles, 960 total presentations and eight readbacks, passed with no shutdown crash.
- Foreground NR/scope-input/FG-toggle/Insert run passed: 1,800 frames, 1,760 foreground, 1,360 two-present reports and 440 one-present reports, totaling 3,160 SDK presentations. Foreground windows measured 100.70/201.40 rendered/presented FPS with FG, 114.94/114.94 off, and 100.40/200.80 after resuming. These are synthetic rates, not gameplay FPS or physical scan-out.
- NR dispatch and elapsed GPU work were logged without the obsolete helper. The logs confirm standard D3D11 teardown, native FG release, retired NR owner reclamation and completed D3D12 runtime shutdown.
- Readback images visibly show the Insert menu and real rendered/presented counters, including about 97.7/195.4 with FG. Menu visibility logs confirm open/close changes. The test temporarily enabled ShowFps; the delivered baseline keeps its default/optional setting.

Failures are preserved: the pre-fix DLL crashed in device-specific shutdown with NR both on and off; diagnostic alias and an initial final-build run lost foreground focus and failed the strict generation threshold. They are not counted as generation passes. A subsequent standalone foreground run passed as reported above.

A sequential old/new sanity check found no obvious slowdown in its valid foreground windows: old 71.61/143.21 versus new 74.88/149.77 with FG; old 88.74/88.74 versus new 91.56/91.56 off. The later new-build interval lost focus, so that entire comparison run failed its generation threshold and its resumed interval is excluded. Variable system load and these limited synthetic samples do not establish a speed improvement.

Verdict: use this patched companion for an isolated upgrade trial; do not substitute either stock release DLL. Core NR + engine FG/menu/counters are probe-verified with the unchanged Test 8 engine integration. The user has independently confirmed Test 8 normal MO2 startup works with the prior companion, not yet with this update in gameplay. This fork is a prerelease; scope image quality, all new NR modes and long-session stability remain unverified. The menu inherits the fork's internal `10.0.0-dev (f45ccf3)` label; the release tag is 0.8.91.
