# Test 8: normal-launch FG capability

Normal desktop shortcut to MO2 to Play now attempts FG capability initialization
without SQA_DLSS_FG or SQA_DX12_BRIDGE. The saved Video FG/PiP switches still
control generation, default Off. The environment variables are no longer read
by game startup. Existing Apply/Cancel/persistence, NR companion, optical-map
scope inputs, wearable protection, cap semantics and telemetry ABI are retained.

CreatePersonalPresentation is shared by engine startup and the standalone probe.
Unsupported display formats/exclusive mode, missing/untrusted runtime, failed
interop/device initialization or unsupported SDK capability leave a native DX11
swapchain after orderly cleanup. Non-NVIDIA adapters are rejected before loading
NVIDIA plugins. Native fallback restores the existing D3D11 maximum-frame-latency
setting. Saved user choices are not rewritten. Runtime failure remains guarded;
the existing -dxgi_old option is an optional native recovery route, not a normal
launch prerequisite. Restart-only SDK lifecycle constraints remain in effect.

Status/tooltips now describe automatic startup and capability failures rather
than asking for a launcher. The settings script is unchanged. The text file is
mirrored in gamedata and extras/dlss_gamma. Matching loose MO2 files must still
win; packed data cannot override an older loose XML/script/shader.

Validation pending: full build; shared automatic-startup helper with both legacy
variables absent; capability-ready/FG-off presentation; missing-runtime and
unsupported-format native fallback; NR/FG off/on integration; existing CPU/GPU,
telemetry/settings checks. Physical unsupported GPUs and the user's actual MO2
gameplay launch are not automatically established by standalone tests.

Delivery preserves the clean two-package layout: runtime contains only bin/db
payloads, including the user-supplied NR runtime and no launcher. Source/licenses,
configuration, manifests, diff and instructions belong to the separate merge kit.
The compiled engine remains SQA-based; non-SQA recipients must port and rebuild.
Test 7 is retained for rollback. No public engine/release/site/installed edits.
