# Personal OptiScaler Menu Fix Test 1 — 18 October 2026

Experimental companion for the existing personal Test 4 engine/settings. Menu operation is verified in the isolated GPU harness; full FG regression and gameplay remain unresolved. Do not treat this as a fully validated replacement for Test 4's compatibility baseline.

Build: personal branch commit 8bb4ecda3c9627f3e88893cc3e02c6e0a62a98c9, successful GitHub Actions run 39119610287. Exact upstream base 5019c979656225ec87e3331658f8a1f3ea478244 of wilsjo2/OptiScaler-DLSSNR-PreSR-Multipass. No engine binaries or configuration are changed by this companion.

## Confirmed

- Original legacy menu received a null output resource after a successful DX12 upscale, so it returned before polling Insert. User confirmed the failure occurred during active gameplay.
- Typed-resource retrieval now falls back to the pointer slot used by the DX11/DX12 bridge. Hidden-menu temporary-copy resource state is restored correctly.
- Interactive harness on RTX 5080: Insert opened and closed the actual OptiScaler window. Captured NGX output visually verified the settings window, not merely a key event.
- Test-only F8 binding switched Neural Rendering off and on live; logs confirm both transitions. This binding is not shipped as an INI change.
- Uncapped interactive runs completed 960 calls, seven resizes and eight readbacks. Generated presentations were 596 with diagnostic logging and 604 with normal logging. Sample-120 results at all eight sizes in the latter run were presented=2/status=0 with advancing input fences. No fence timeout occurred.
- Separate 60-render-FPS interaction run also opened/closed the menu and toggled NR, but counted only 16 generated presentations.

## Failed checks and limits

All these runs missed the existing strict >=900/960 generated-presentation threshold and exited 9. The threshold was not weakened. Original unpatched DLL also missed it under current conditions (204 in a focused interactive run; 120 in a non-interactive-key run). Patched DLL's non-interactive-key run counted zero. Focus, display metering and test pacing may contribute, but the cause is not established. Prior Test 4's 953/960 result is historical evidence, not a passing result for this DLL.

The harness presents a synthetic target and captures the actual upscaler output separately for menu inspection. It does not reproduce engine rendering, wearable composition or actual game input routing. Mouse-based settings editing/saving, backend hot-switches, sustained FG, game performance, wearable edges and gameplay stability remain unverified. Keep the fixed DX12/NR backend initially; test FG off/on in a disposable game session. Startup-only settings still require a restart.

## Files and source

Only bin/dxgi.dll is deployed. Back up the old DLL, retain the Test 4 INI (FrameGen.External=true, Menu.OverlayMenu=false), NR model and dependencies, and restart once. Restore the original DLL and restart to roll back. Original engine/settings ZIPs remain unchanged.

DLL SHA256: A3CEEA403A91DD679095A2271151D2ECB9631E39F75EF2B76526E4F4705C98A7.
Separate corresponding-source ZIP: Squared Away Personal OptiScaler Menu Fix Test 1 Source.zip.
Source SHA256: 501B0EAFFE9C89C895E70CD7A8A3587A907B02FDA50E35BDA8EF349FE66CA719.
Source includes patched tracked files, recursive submodule contents and upstream license/notices. Both patched C++ files were hash-checked against the build preparation worktree.

Evidence: C:/GAMMA/sqa-previews/personal-frame-generation/probe-menu-fixed (uncapped-evidence, paced-evidence, fg-only-evidence and final normal-logging run), probe-menu-baseline, probe-menu-diagnostic and diagnosis-menu-20261018. Probe commits b1e5b898 and ef6c6460; source patch build commit remains 8bb4ecda.

Installed reads were read-only: C:/Anomaly/bin/OptiScaler.ini and OptiScaler.log supplied by the Full Pack's OptiScaler; C:/Anomaly/appdata/logs/xray_fomok.log from the personal engine. No installed writes or game launch. Public engine remains 93483f8a0211bec6583af6179a0038dc6633dfce.
