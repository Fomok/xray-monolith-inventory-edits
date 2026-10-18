# Personal OptiScaler NR 0.8.91 compatibility companion

Current source is the standard NR fork release v0.8.91 at
https://github.com/wilsjo2/OptiScaler-DLSSNR-PreSR-Multipass/tree/f45ccf3a761df91450959d325ac0166cba5364c9
with menu-compat.patch. See docs/PERSONAL-OPTISCALER-NR-0891-VALIDATION.md for
the release comparison, source/build provenance, tests, failures and limits.

Keep FrameGen.External=true, FrameGen.Enabled=false and Menu.OverlayMenu=false.
The personal Test 8 engine initializes normally through MO2 and owns FG/PiP,
Streamline and Reflex. This companion preserves measured rendered/presented
counters, legacy menu state restoration and startup-only external ownership.
It uses standard D3D11 NGX shutdown in external mode to avoid the reproduced
device-specific shutdown fault, then drains NR during normal D3D12 teardown.

Build the pinned source with recursive submodules, apply menu-compat.patch,
and build OptiScaler.sln Release/x64. The personal-optiscaler workflow also
runs the NR diagnostics regression and archives complete corresponding source.

Use the separate small bin replacement ZIP plus source/license ZIP. Retain
Test 8 engine/Streamline/SR/NR model files. Back up the old DLL, INI and helper;
move obsolete nvngx.dll_dlssnr.dll out of bin. Keep nvngx_dlssnr.dll, the actual
NR model. The new fork owns NR directly and no longer needs the old helper.
Restore all three backed-up companion files to roll back. Do not replace the
patched companion with a stock official release: it lacks this NR integration.

The prior 0.7.6 companion remains in the preserved Test 8 packages. Earlier
diagnosis and validation are historical:

# Personal OptiScaler legacy menu companion

This companion targets the exact DLSS-NR fork supplied with the GAMMA Bodycam
0.3 + DLSS + DLSS 5 Full Pack. Base source:
https://github.com/wilsjo2/OptiScaler-DLSSNR-PreSR-Multipass/tree/5019c979656225ec87e3331658f8a1f3ea478244

The dual-counter companion requires the matching personal telemetry engine. Keep `FrameGen.External=true` and
`Menu.OverlayMenu=false`; enabling the separate presentation overlay reproduces
the earlier NR/FG compatibility failure. The companion corrects the legacy menu
instead of re-enabling that path.

## Reproduced cause

The bridge writes shared D3D12 resources into an NGX parameter table via `void*`.
With the game's D3D11 table, the typed D3D12 getter returns null. Diagnostic logs
showed successful upscaling at frames 31/60, a valid menu object, and a null output
at `Menu_Dx12::Render`. That function returns before input polling or drawing.
Changing Insert or input mode cannot fix the missing texture.

`menu-compat.patch` adds an untyped fallback for output, motion and depth retrieval
in `IFeature_Dx12::Evaluate`, retaining the native typed path. It also restores the
output's UAV state when the temporary menu render-target path draws nothing.
The latter fixes an unbalanced COPY_SOURCE transition when the menu is closed.
Bounded diagnostic messages identify the resource handoff in test logs.

No engine rendering, wearable composition, Streamline queue ownership or timeout
is changed. No native presentation-overlay hook is re-enabled.

## Rebuild

Check out the pinned source with recursive submodules. Apply `menu-compat.patch`
with `git apply`, then build `OptiScaler.sln` using Visual Studio 2022 C++ tools,
Release/x64. The workflow `.github/workflows/personal-optiscaler.yml` performs
these steps. Its build manifest records the personal patch commit, base commit
and DLL hash. The corresponding-source archive includes patched sources and
pinned submodule files; retain the upstream GPL-3.0 license and third-party notices.

## Deployment and rollback

Quit the game. Back up the existing test game's `bin/dxgi.dll`, then copy the
companion's `bin/dxgi.dll` there. Keep the existing NR model, loader, dependencies
and Test 4 INI. This is not an MO2 mod and does not replace the engine package.
Restart once to load the new DLL. Insert should open/close the legacy settings
window during active upscaled gameplay. Live settings retain the fork's normal
behavior; startup-only settings still require restart.

Restore the backed-up DLL and restart to undo the companion. Preserve the original
Test 4 engine/settings ZIPs as the known rendering baseline. Validation results and
limitations must accompany the final package; build success alone is not menu proof.

## Current validation
The Dual FPS companion requires the matching personal engine telemetry build. Overlay and Insert menu show Rendered and measured Presented FPS, with half-second windows and explicit measuring/stale/unavailable states. No multiplier or extra SDK query. See docs/PERSONAL-FG-DUAL-FPS-VALIDATION.md for on/off/resume GPU captures and protocol edge cases. The older Test 2 A/B results remain in VALIDATION.md as historical comparison evidence. Physical scan-out and game visuals remain unverified.
