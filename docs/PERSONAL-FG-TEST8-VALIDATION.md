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

## Verified

Source fa1b97c3c85bd2bfb1704dc937e2fe7ce4d09169 built successfully in
[run39147479433](https://github.com/Fomok/xray-monolith-inventory-edits/actions/runs/39147479433),
artifact12144726128, digestsha256:85efbd6437d7457a243a91ed496f644d00f952047a7c5bd1a1db7c5095e2827a.
All30 CI file hashes and six NVIDIA runtime signatures verified.
Four CPU regressions passed. Settings LuaJIT checks retained availability,
Apply/Cancel/persistence-wrapper behavior, status and no-restart flags. GPU
scope/depth/motion/UI and dual-counter telemetry probes exited0.

The engine and probe use the same CreatePersonalPresentation startup helper.
Both legacy environment variables were absent and the probe asserts this:

- Supported startup loaded capability and presented12frames with FG OFF.
- Missing Streamline runtime fell back to native DX11 and presented12frames.
- Unsupported10-bit display format fell back and presented12native frames.
- Missing sl.dlss_g.dll allowed slInit, then SDK support returned31; orderly
  cleanup fell back to native DX11 and presented12frames. No crash/retry loop.

The first capped1800-frame NR/scope/FG-toggle integration run lost foreground
after340frames and FAILED its strict generation threshold (exit9,328two-frame
reports). This result is preserved; it is not counted as a pass.
An uncapped, shorter-in-time retry completed1800frames and PASSED (exit0),
1760foreground frames,1337two-frame reports and3137SDKpresentations. NR dispatch
was logged. Foreground windows200..400 measured203.195rendered/406.390DXGI FPS;
600..800 with FG off255.987/255.987;1100..1500 resumed201.867/403.733.
These are synthetic probe measurements, not gameplay FPS or a speed comparison.

The settings still default Off and preserve saved choices; capability setup
does not enable interpolation. Existing PiP mapping, NR, UI protection, pacing
logic and telemetry protocol are unchanged. Physical non-NVIDIA/unsupported
GPU behavior is source-guarded, not hardware-tested here. Actual normal MO2
gameplay launch, packed DB loading/loose-file precedence, in-lens image quality
and long-session stability remain user validation. No installed files changed.

## Delivery

Runtime ZIP has14files under bin/db only, including the supplied NR310.8 runtime,
patched companion/INI, and no launcher/probe/PDB/source/docs. The separate merge
kit carries sources/licenses/corresponding source, config, diff and manifests.
The base DB is preserved from Test7; the stored-format supplemental DB contains
the three current UI/shader files and is independently index/payload/CRC checked.
Only the XML changed since Test7. Loose MO2 copies still override packed files.
The compiled engine remains SQA-based; non-SQA recipients must port and rebuild.
Test7 runtime/merge ZIPs remain for rollback. No public branch/release/site edits.
