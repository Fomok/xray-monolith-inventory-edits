# Personal Test 4 validation — 2026-10-18

Engine/probe build: ed3b529102e6e26934de9026920c6221df6d591b, successful
[Actions run 39117134344](https://github.com/Fomok/xray-monolith-inventory-edits/actions/runs/39117134344).
Engine changes are a2bc697a; subsequent commits strengthen the standalone probe.
Four regression executables, full DX11-AVX build, matching gamedata and packaging passed.
CI artifact 12132484790 SHA256: 985661BEBBCDE1700D6F826CEA69C98677B7CB76429C40D3927C4E0F27D701C3.
All 26 manifest files and six NVIDIA runtime signatures verified locally.
The delivered ZIP uses these unchanged binaries/gamedata, plus the corrected INI
helper, instructions and this report from the packaging commit recorded in its manifest.

## Hardware checks

RTX 5080. The visible, message-pumped `bridge_probe --streamline --ngx --generate`
uses synthetic DLAA color/depth/motion, a synthetic UI rectangle, and eight sizes
from 1280x720 to 1504x944. It drains between seven resizes/NGX feature recreations.
It requires at least 900 generated presentations. This is not gameplay validation.

| Configuration | Result |
|---|---|
| Native NGX + Streamline, FG off | 960 calls, seven resizes, eight readbacks; pass |
| Native NGX + active FG | 960 calls, 953 generated presentations; pass |
| Full Pack fork NR + SR, FG off | NR dispatch confirmed at all sizes; pass |
| Same fork, External=true, default overlay, active FG | Fence stall and crash; failed |
| External=true, factory wrapping enabled, active FG | Still crashed; failed |
| External=true, legacy menu, other overlays blocked | 960 calls, 953 generated; pass |
| External=true, legacy menu, other overlays left alone | 960 calls, 953 generated; pass |
| Final helper output from untouched Full Pack defaults | 960 calls, 953 generated, seven resizes/eight readbacks; pass |

The final configuration changes only `FrameGen.External=false` to `true` and
`Menu.OverlayMenu=auto` to `false` relative to Full Pack defaults. The helper also
selects `Upscalers.Dx11Upscaler=dlss_12` and `DlssNr.Enabled=true` (already defaults).
Other overlay-blocking and factory-wrapping changes are unnecessary for this pass.
The DLL is the supplied wilsjo2 fork, commit 5019c979656225ec87e3331658f8a1f3ea478244,
SHA256 CF9C59158477B84A414334819E07A961530FF1308C0BB9FD3849CEB2A276C01E.
Its binary is unchanged. Official OptiScaler lacks this fork's NR path.

Final OptiScaler log confirms exposure supplied, NR before SR at all eight sizes,
and no missing-exposure recreation or fence timeout. Log SHA256:
2B9D0F27E44515457DE99593602E712E5ADB1A514B5104ACCFC616F26CAB0810.
Result report SHA256: 62C84CA929124A5F569AFF21E51E4A9A2EA40222CEFCA55EA50D61CCD50237EF.
Evidence is preserved locally in
`C:/GAMMA/sqa-previews/personal-frame-generation/probe-a2bc697a-optiscaler/`
as `OptiScaler-final-settings-active-pass.log` and `result-final-settings-active-pass.txt`.
The tested probe hash matches the CI package's probe. Earlier failed logs/dump remain
beside these files; no timeout was relaxed and no synchronization fence was removed.

GPU input tests passed depth/motion conversion, solid-widget and translucent-glass
mask regions, dilation/background checks, unchanged clean-scene pixels and context
restoration. Wearable rendering itself is an engine integration pending gameplay.

## Interpretation and remaining tests

The default overlay path reproduces a compatibility failure; legacy mode resolves
it in this standalone workload. A captured-queue dependency cycle was investigated
but not proven. This does not establish the exact command behind the earlier game
crash. Its 0x887a0006 could have been synthesized by the old engine's CPU fence timeout;
Test 4 now logs the actual DX12 removal reason separately.

Test worn Promin/Vektor readability, native depth/hand/glass occlusion, movement
edges and frame times in a disposable game profile. Compare FG off/on and NR off/on
with a fixed backend first. Menu interaction, live backend changes, long sessions
and image quality are not covered by the probe. Seven nongenerated presentations
are the first warmup after each resize. SDK timing/alignment/repeated-option
warnings occurred in passing synthetic runs and are retained in logs.

Installed reads only: OptiScaler dxgi.dll hash and saved log/config; personal engine
game log; MO2 profile; Wearable Devices attachment/UI scripts and prior documented
comparison scripts. Archive/dependency copies were placed only in the probe workspace.
Public engine, installed files, weapon-follow and pivot experiments were unchanged.
