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
