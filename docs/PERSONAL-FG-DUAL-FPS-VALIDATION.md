# Personal dual FPS — 18 October 2026

The personal engine publishes measured application/presentation totals to the matching OptiScaler companion. The overlay and Insert settings window show Rendered and Presented FPS. Presented includes reported generated frames; it is not monitor scan-out FPS or proof of distinct interpolated imagery.

## Implementation

Built engine/companion source:7c3ba654617c0713b181395287deaf82f664ed5c. Visual probe:9d85880b37014e3de66803a1040d45a3c41660e0. Engine changes are limited to PersonalFgTelemetry.h and StreamlineIntegration.h/.cpp. The existing normal post-Present slDLSSGGetState result supplies numFramesActuallyPresented; no extra SDK queries are made. One application count is added per normal query, presentation counts are accumulated as returned. Invalid/sentinel/error states are not treated as valid zero rates or guessed multipliers.

Same-process, PID-scoped Local/SQA_PersonalFG_v1 mapping:64-byte versioned ABI, sequence-guarded snapshot, QPC timestamps/cumulative counts, producer read/write and consumer FILE_MAP_READ. A half-second delta window computes both rates from the same interval. FG mode changes reset the window; gaps over one second create a new epoch; stale or missing/invalid data clears the displayed rate. No queue waits, resource-lifetime changes, telemetry threads, network, administrator requirement or rendering changes. Mapping failure only makes the counter unavailable.

OptiScaler keeps the functional legacy menu and prior resource fixes, External=true/OverlayMenu=false. menu_common.cpp uses the reader for both overlay and menu; ordinary application timing remains available as a fallback. The protocol header is identical in engine and companion (SHA2560383B11A3553C836511455E7FB46308F5EF157935A7D485022F00F1153E938A8). Existing wearable composition, scope/vision guards, NR/DX12 compatibility settings and rollback behavior remain.

## Validation

Telemetry self-test passed in CI and locally: missing producer; measured120/120 off; variable1/3 presentation counts producing120/240; enabled with zero actual counts produces zero Presented; mode resets; stale data; recovery; invalidation; fresh epoch after a gap even if reader did not observe the pause. Tests use explicit exit failures, not disabled release assertions.

Actual RTX5080 GPU visual test, matching companion, DX12+NR,1800frames: pass with1344 generated-report calls (400 frames deliberately suspended plus startup/transitions),3144 total reported presentations and successful shared-target readback. Insert and NR toggles retained. Captured actual NGX menu images at frames450/750/1050 were visually checked:

- FG on: Rendered204.1 / Presented408.2.
- Suspended:245.2 /245.2, labelled FGoff/suspended.
- Resumed:201.7 /403.4.

CSV reader snapshots independently record SDK2,1,2 and warming states exactly at transitions500/900. The menu consumer and CSV consumer begin their half-second windows at slightly different frames, so their instantaneous rounded values need not match exactly.

An earlier no-menu toggle run reported only one frame after re-enable; the counter correctly showed equal rates despite the enabled flag (e.g.252.692/252.692). That run failed its sustained-generation threshold, retained as evidence; no false2x display or claim of a fixed runtime-resume issue. The subsequent visual toggle run resumed2x and passed. This feature reports measured output without concealing such conditions.

Old-engine/new-companion test visibly showed App189.6 FPS / Presented unavailable, while Insert remained functional. Its legacy resize-generation threshold failed591/960 as previously observed; this test validates missing-telemetry behavior, not sustained generation. Stale/invalid handling was verified through the exact shared reader in deterministic tests; no claim of a photographed loading-screen stale label.

Evidence: C:/GAMMA/sqa-previews/personal-frame-generation/dual-fps-20261018, including engine-telemetry-toggle.csv/log, dual-ui-toggle.csv/log, menu-frame-450/750/1050.bmp and old-engine-unavailable.bmp. No installed game files read or written for this feature; only workspace sources, copies and saved prior evidence. No game launch/admin tracing. Gameplay and physical displayed-FPS/image-quality confirmation remain user testing.

## Build/source provenance

Companion run39125933371 succeeded. DLLSHA256F16801A098CF5242F57989C8C38CFF1206A3D4DD4CB64DF28EB0A8A9B1B74628. Exact upstream wilsjo2/OptiScaler-DLSSNR-PreSR-Multipass5019c979656225ec87e3331658f8a1f3ea478244. Corresponding source contains all patched sources, new protocol header, pinned recursive submodules and licenses. Current CI archive omitted newly added untracked header; packaging adds the exact built header, verifies all four modified/added entries, and the workflow is corrected for future archives. Final source SHA256334A97A0BA505F4FD3DA6DAE7A59C02AA02884F7072B12D54D0C7E72C77B3CD9.

Probe run39125996687 succeeded, including protocol tests. Full engine build/package verification is recorded in the final delivery manifest and accompanying validation addendum.

## Completed engine build
Full DX11-AVX run39125933391 succeeded at7c3ba654. Complete artifact12136298942 SHA2561058BE25AFAB094130C5D7927222476661E5E0659A4DCD9B94DA0485EEF8A422. All27CI manifest files verified; six Streamline/NVIDIA runtime signatures valid. Existing four CI regression programs passed. Delivered bin/db content remains hash-identical to CI; documentation/installation metadata is updated for Test5 DualFPS. Public branch remains93483f8a0211bec6583af6179a0038dc6633dfce.
