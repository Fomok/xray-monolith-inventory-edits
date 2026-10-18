# Menu Fix Test 2: controlled comparison, 18 October 2026

No menu-patch FG regression was reproduced in the matched synthetic workload. A confirmed reporting defect was corrected: legacy external-FG timing is now explicitly application FPS, using its own clock so optional swapchain timing callbacks cannot contaminate it. Menu graph and overlay identify external FG as excluded. No automatic doubling, engine changes, queue/fence changes, or overlay re-enabling.

## Historical comparison

User clarified both ~300 before and ~160 after came from OptiScaler. Saved11:28 session used original Full Pack DLL, ordinary DLSS, External=false, OverlayMenu=auto (default true, Config.h:631); NR did not run. Later DX12 switch crashed. Saved12:48 session used menu-fix DLL, External=true, OverlayMenu=false, initially DX12+NR then ordinary DLSS. NR cost3.99–4.48ms; native1920x1080 DLAA in both backend initializations. This was not a DLL-only A/B.

Original presentation-overlay timing runs through wrapped_swapchain.cpp:375, menu_overlay_dx.cpp:528 onward and MenuCommon::UpdateRenderTiming's OverlayMenu branch. Current legacy timing runs during upscaler evaluation. External mode forces OptiScaler-owned FG to NoFG in dllmain.cpp:1867–1882. Neither historical screenshot/counter sample is available to establish exact physical output rate. The change of measurement point is confirmed; a precise300->160 causal decomposition is not.

12:48 game log already shows sampled SDK presented2 with current tags/status0, explicit FGoff->1 and on->2, scope suspension/resumption and no persistent reset. That alone did not settle display regression, so the controlled tests below additionally query DXGI statistics.

## Controlled setup

Exact original DLL SHA256CF9C59158477B84A414334819E07A961530FF1308C0BB9FD3849CEB2A276C01E versus first menu-fix DLL A3CEEA403A91DD679095A2271151D2ECB9631E39F75EF2B76526E4F4705C98A7 versus final5FEA2DF22AEB21844B97C0CB540C035F022B92641C1133F83533FA1D1694EE44. Same engine bridge/upscaler sources, NVIDIA dependencies, matched INI, synthetic inputs, visible foreground window,1280x720 DLAA, fixed backend and uncapped rendering. Tests use workspace copies only.

Probe479b9d36 records per-query SDK count, QPC, foreground, GetFrameStatistics and GetLastPresentCount. Probe26783107 adds1800-frame fixed-resolution mode; legacy960-call/7resize test and its>=900 threshold remain. Steady threshold1620/1800; measured settled window is frames200–1799, not selected by outcome. Full CSV retained.

| Backend / DLL | App FPS | DXGI presentations/sec | DXGI count /1599 intervals |
|---|---:|---:|---:|
| DX12+NR original |214.93|429.85|3198|
| DX12+NR first menu fix |214.57|429.15|3198|
| DX12+NR corrected |212.65|425.31|3198|
| Ordinary DLSS original |582.48|1164.96|3198|
| Ordinary DLSS first menu fix |580.12|1160.24|3198|
| Ordinary DLSS corrected |584.03|1168.05|3198|
| DX12+NR original, FGoff |264.44|264.44|1599|
| DX12+NR first menu fix, FGoff |262.89|262.89|1599|

Every settled on sample reports SDK2; every off sample SDK1. All steady runs exit0 and pass readback. Entire1800-frame DX12+NR on runs:1776/1777/1778 generated-report calls for original/menu-fix/corrected. Ordinary DLSS all1680. Repeat with ShowFps=true: application214.06/212.08/212.45FPS respectively, all3198 DXGI counts/1599 intervals, all settled SDK2; total generated1776/1775/1776. Small timing differences are single-run variation, not a statistically established performance effect.

## Earlier inconsistent probe resolved to a narrower limitation

Matched960-call resize runs: original629 versus patched631 generated-report calls; SDK totals1589/1591 and DXGI cumulative counts1562/1565 (asynchronous final counter lag expected; not asserted equal). All last40 samples of each of8passes report2:320samples/640SDKframes for both DLLs. One-frame intervals cluster before settling after repeated feature recreation. The separate steady test establishes sustained generation for both; it does not make the resize test pass. Final corrected interaction test still exits9 with632/960; this failed strict resize criterion is retained. Earlier hypothesis that0/4 asynchronous SDK reports caused the low binary count was not supported: observed distribution was1/2. Exact transient cause remains unlocalized.

## Independent measurement limits

DXGI counts independently confirm approximately2:1 presentations versus app iterations, not optical scan-out, visually distinct interpolated pixels, or every frame reaching the display. Refresh counters advance at about100Hz; Windows video-controller query also reports current1920x1080/100Hz. This is current desktop evidence, not proof of historical game display settings. Presentation rates above100 are not that many full monitor refreshes. The probe presents synthetic color/UI rather than the NGX output; captured NGX images separately verify the menu. Actual scene motion, wearable edges/masks, game pacing and perceived smoothness still need game observation.

Official portable PresentMon2.6.0 failed to start ETW with access denied. User chose to continue without administrator tracing; no UAC prompt/admin capture or system membership change. Reference: https://github.com/GameTechDev/PresentMon/blob/main/README-ConsoleApplication.md .

## Final correction and delivery

Build495d2e87b681cc33b544442778cfd4874b4ea3f5, successful run39121307982. Insert open/close and live test-only F8 NRfalse/true confirmed in logs; actual captured menu/overlay images visually show the application-FPS label. No test key/INI change shipped. Corresponding-source ZIP includes recursive pinned submodules and license; all3patched C++ entries hash-verified against preparation worktree. Artifact hashes verified before extraction. Source SHA25657D1AFA50957F630349F67696DDD2C179EA812D5254B190022A5CF69210BFFC6. Binary SHA2565FEA2DF22AEB21844B97C0CB540C035F022B92641C1133F83533FA1D1694EE44.

Evidence root C:/GAMMA/sqa-previews/personal-frame-generation/menu-ab-20261018; comparison.csv plus full per-frame CSVs and logs. Historical snapshots diagnosis-test3-20261018-1128 and diagnosis-menu-game-20261018-1248. No new installed game reads in this comparison; reused preserved personal-engine log and Full Pack OptiScaler logs/config plus workspace DLL copies. Previous diagnosis read installed dxgi.dll hash. No installed writes/game launch; public engine and frozen weapon-follow/pivot untouched.

## Subsequent dual FPS companion
The matching personal engine/companion now reports measured Rendered and Presented rates. See docs/PERSONAL-FG-DUAL-FPS-VALIDATION.md. The above describes Test 2 and remains historical evidence.
