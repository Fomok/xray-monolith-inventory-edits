# Personal FG Test 6: PiP inputs, settings and cap diagnosis

## Scope and implementation
Personal branch only: Fomok/xray-monolith-inventory-edits, personal-bodycam-dlss-frame-generation. Engine source195a81e4f6141e2341beafd1006039c5590de1e0; integration probe7223a34996a06ecc77e512165b5282c6ae0a19a9. Public amp-bodycam-dlss-mt remains93483f8a0211bec6583af6179a0038dc6633dfce.

Iron sights are eligible without treating the global3D-scope feature switch as a visible lens. Physical hip glass is not treated as an active second camera. Independent saved r_dlss_fg_pip defaults0; Off suspends FG while scoped. On supports current Bodycam Reference optical images with a matching current scope capture and mapping; legacy/unmapped optics, NV/thermal and existing unsafe embedded UI remain excluded.

PersonalFgScope.h / PersonalFgInputs.h use scope-camera depth and stored motion, or the scope's camera reprojection for missing vectors. They invert the previous optical capture-coordinate map to transform motion into final lens-image coordinates; retain the main-world inputs outside the mapped lens; convert scope linear depth into the main depth encoding; remove main raster-jitter delta; preserve deferred-glass occlusion in the stored map. Newly exposed/unsolved pixels get zero motion for optical-flow correspondence instead of unrelated main-camera motion. The solver is bounded; fast/noninvertible optical changes remain a visual test risk.

Scope color remains in HUD-less color and is NOT covered by a native-cadence UI mask. The normal whole-image DLSS-G call interpolates it with the transformed guides. This is composite-input generation through one presented viewport, not a claim of two separate NVIDIA FG instances. Scope image/reticle color, Bodycam optical kernel, SR view history and native wearable/UI composition are preserved. The optical shader adds a coverage channel to its existing capture map, retaining the prior XY and W contract for reticle light. History resets on entry/exit, optic/session/camera epoch, zoom, render-size and camera cuts. Invalid/missing current inputs fail closed.

The NVIDIA integration guide documents independent viewport tags and distortion handling; arbitrary overlapping optical meshes are not equivalent to simple rectangular subviews. This implementation transports the actual optical sampling map into the final image instead of passing raw scope-camera guides under main-camera coordinates. Reference: https://github.com/NVIDIA-RTX/Streamline/blob/main/docs/ProgrammingGuideDLSS_G.md .

## Settings
Both gamedata/scripts/modxml_dlss_options.script and extras/dlss_gamma companion expose master and PiP switches beside upscaling. Numeric token strings preserve existing0/1 console/config compatibility. Standard pending Apply/Cancel and cfg_save paths are retained; no vid_restart requested by FG switches. r_dlss_fg_status is a read-only nonpersistent capability query for ready/restart/unavailable text. Runtime opt-in SQA_DLSS_FG=1 still occurs at process startup; toggles alone cannot load it into an already-native process. The runtime FPS overlay shows actual active/suspended state rather than implying a saved On preference guarantees generation.

Read-only installed source comparison: the existing DLSS FSR3 Compatibility Patch modxml options script matches the pre-change repository version by lines. GAMMA UI ui_options.script confirms standard command persistence through cfg_save. No installed modifications.

## Confirmed cap semantics
Probe commit e28c3ed874d0de812014ba52f98ddbbbda4a9eea, successful build39130219377, artifact12137472444 digest e19241e7fc3087c1f54ca2e51d98417e36db7c0143d278822e70c1d21f500b22. Reflex frameLimitUs10000:
- Foreground frames100-300:49.99978 rendered/s,99.99956 DXGI presentations/s, SDK2 on all201 samples.
- Foreground frames600-700 with intentional FG suspension:99.97073 rendered and presented/s, SDK1 on all101 samples.
Thus r__framelimit100 is an output target with FG. Preserve the user's value and behavior; correct the prior inaccurate rendered-cap comment. Lower GPU usage at50 rendered can be expected. Full probe exit9 followed focus loss, so only verified foreground intervals establish these measurements. Evidence: diagnosis-test5-20261018/cap-focused.csv and cap-measurements.json. No driver cap or user config changed.

## PDA recording
67.82-second user recording: NR switched on around18s before PDA~24-32s; FPS lower throughout, NR off38.8s restores it before scaling changes41-46s. No persistent PDA defect established; no speculative PDA patch. Full timeline/video hash: diagnosis-test5-20261018/video-analysis.md.

## Validation completed
- GPU readback on RTX5080: scope first-frame/reset, moved optical map with independent object motion, separate scope-camera fallback, depth encoding, occluded/revealed lens, scope exit, unaffected main view, UI/glass protection/dilation and D3D11 context-state restore PASS. Initial exact-zero test caught a filter residual~0.0016pixel; final tolerance is below0.004pixel at the64px test size, while the independent horizontal motion remains exact. No production failure was hidden by removing the motion assertion.
- LuaJIT2.1 harness: syntax, single insertion, absence on unsupported engines, pending changes/cancel/apply wrapper contract, status text and no vid_restart flags PASS. XML parses32strings. In-game layout/interactions are not yet tested.
- Integration probe7223a349 built39131921981: mapped scope guides through actual DLSS-G with existing dual-counter OptiScaler/NR dependencies. Foreground frames200-400:195.82998 rendered,391.65995 DXGI presentations/s; all201 SDK samples2. Full1800-frame run exit9:393 generated reports, with1385 samples outside foreground. One repeat also lost foreground focus (1542 nonforeground samples,237 generated reports, exit9). Sustained threshold did NOT pass in either run. No claim of sustained in-game interpolation or visual quality.
- No admin PresentMon tracing performed, per earlier user preference.

## Delivery evidence
Full engine CI39131776042 succeeded at source195a81e4. Artifact12139190402 digestsha256:2333f4d1dd4b32cee9da1cbeed5e12671660a67828566af0eb1581f1a40ba507. All28 original manifest entries and6 NVIDIA signatures verified. Engine bin/db files remain byte-identical to CI. Updated installation/validation prose is accompanied by the original CI manifest and a new delivery manifest.

Installed reads: personal engine log/user.ltx; Full Pack OptiScaler log/INI; MO2 selected profile/modlist; GAMMA UI ui_options.script; DLSS FSR3 Compatibility Patch options script. Only workspace copies/reports/packages changed. Public engine and frozen weapon-follow/pivot work untouched.

Final delivery ZIP hashes (verified after extraction):
- Engine:116657863bytes;SHA25639167F1367DE0478B5539A087287326891B108AEC865B9F442093DD69CBCA1C1.
- UI/scope shader:14652bytes;SHA256622E44C44803656B5AF6D97E4A86171A655E6960530081BF91D6F51B8880B92C;6manifest entries verified.
Both under C:/GAMMA/sqa-previews/personal-frame-generation/. Keep existing OptiScaler Dual FPS Test1; the new UI/shader mod must win DLSS compatibility and Bodycam overrides. Complete rollback is documented in each ZIP.
Source: https://github.com/Fomok/xray-monolith-inventory-edits/commit/195a81e4f6141e2341beafd1006039c5590de1e0 . Build: https://github.com/Fomok/xray-monolith-inventory-edits/actions/runs/39131776042 .
