# Personal Bodycam + DLSS frame generation

Checked 2026-10-18. Real DLSS 2x frame generation is implemented on the personal branch. The full build and package verification passed; generated frames and gameplay quality remain unconfirmed. The earlier investigation and bridge-only prototype below are historical context, not the current implementation status.

## Personal FG implementation

`SQA_DLSS_FG=1` loads the optional Streamline 2.14.16 runtime and DX12 proxy swapchain. `r_dlss_fg 1` requests generation only when inputs are eligible; the console setting defaults off. The native DX11 renderer and DLSS/FSR upscaler remain intact. `SQA_DX12_BRIDGE=1` alone still enables only ordinary bridge presentation.

The integration supplies shared depth, motion, HUD-less color and a conservative UI protection mask, with Reflex tokens/markers and synchronized input retirement. The UI mask is a dilated binary difference, not reconstructed translucent alpha. Scope/PiP, vision effects, embedded UI, menus/loading/pause and unfocused windows suspend interpolation. SDR windowed/borderless only; start testing with VSync off. The first implementation serializes input reuse and may cost performance.

Sources: `src/Layers/xrRenderDX10/{dx12PresentationBridge.h,PersonalFgInputs.h,StreamlineIntegration.cpp}` and `src/Layers/xrRenderPC_R4/Dlss/PersonalFrameGeneration.cpp`, plus engine simulation hooks. Runtime integration adapts Brodrigz tag 1.0.2 with OGSR/xrSimpAI credits. See [provenance](PERSONAL-FG-PROVENANCE.md) and [installation/test/rollback instructions](PERSONAL-FG-TEST.txt).

Local checks passed: SDK capability/Reflex/proxy presentation with generation OFF (180 calls, two resizes); real GPU readback of converted depth/motion and UI protection including context-state restoration on RTX 5080. These do not establish actual interpolation. In-game proof needs enabled diagnostics with actual presented count 2 and successful status, plus visual, latency and frame-time checks.

## DX12 presentation prototype 1

The personal branch now keeps native D3D11 rendering and NGX upscaling, but can present completed frames through D3D12. Enable only by setting the game process environment variable `SQA_DX12_BRIDGE=1`. The default path still uses the original DX11 swapchain. This is not a frame-generation toggle.

Implementation: `src/Layers/xrRenderDX10/dx12PresentationBridge.h`, integrated through `CHW::GetPresentationTexture` and `CHW::PresentFrame`. A D3D12 device is created on the adapter obtained from the actual D3D11 device. A shared simultaneous-access RGBA8 render target is opened by D3D11; a shared fence establishes DX11 completion before the DX12 copy. DX12 transitions the current swapchain buffer for copy and returns it to PRESENT. A bounded CPU fence wait protects reuse, allocator reset and resize. This intentionally serializes frames and is not a performance optimization.

The bridge validates shared texture/RTV and fence support before taking ownership of the window swapchain. Startup failures log an HRESULT and fall back to native DX11. SDR windowed/borderless only for this first prototype; HDR/exclusive startup and `-dxgi_old` retain the native path. An exclusive-mode request while active is held in borderless mode. Runtime presentation or resize failure stops with restart instructions; there is no unsafe hot switch after a failed GPU wait.

The existing main/PiP hidden-pass presentation gate remains in place. R4 PDA/scope capture and asynchronous screenshot reads now obtain the shared presentation texture rather than asking the D3D12 swapchain for a D3D11 texture. No scope/upscaler shaders or frozen experiment code were changed.

Why shared resources instead of D3D11On12: this preserves the native D3D11 device used by NGX, FSR and existing renderer resources. D3D11On12 would also change device behavior throughout the renderer. Compatibility with a future frame-generation SDK is still unverified; this prototype transports only final color, not the HUD-less color, depth, motion or UI buffers needed for FG.

Build: the personal branch's `amp-bodycam-build.yml` builds DX11 AVX, the existing temporal/HUD tests, a standalone hardware probe and matching DB0. No release publication is configured in this workflow. Test instructions are in `docs/PERSONAL-BRIDGE-TEST.txt`.

The hardware probe uses a hidden standalone window, 180 presentations over three sizes and two resizes. A successful probe only establishes API-level transport/presentation, not image quality, gameplay, scope behavior or performance. Runtime and final build evidence will be recorded with the delivered package.

Microsoft resource-sharing reference: https://learn.microsoft.com/en-us/windows/win32/api/d3d11_1/nf-d3d11_1-id3d11device1-opensharedresource1 . Shared simultaneous-access state semantics: https://microsoft.github.io/DirectX-Specs/d3d/ResourceHeaps.html .

## Verified baseline

- Branch: personal-bodycam-dlss-frame-generation.
- Isolated checkout: C:/GAMMA/sqa-engine-personal-frame-generation.
- Base: 93483f8a0211bec6583af6179a0038dc6633dfce.
- Remote amp-bodycam-dlss-mt and tag bodycam-dlss-2026.10.18 both resolved to that exact commit during this investigation.
- Original checkout was clean. No weapon-follow or pivot branch was merged.
- Local NVIDIA query identified GeForce RTX 5080, driver 617.14. This is hardware identification, not a successful SDK capability check; HAGS and runtime feature eligibility remain untested.

## Exact source evidence

Paths below are relative to the baseline repository.

- src/Layers/xrRenderDX10/dx10HW.cpp:567 creates ID3D11Device through D3D11CreateDevice; line 600 supplies that device to CreateSwapChainForHwnd.
- src/Layers/xrRender/dxRenderDeviceRender.cpp:510-518 gates presentation for true PiP / legacy hidden SVP frames and calls the DXGI swapchain directly.
- src/Layers/xrRenderPC_R4/Dlss/DlssD3D11.cpp uses NVSDK_NGX_D3D11 and DLSS evaluation for super resolution.
- src/Layers/xrRenderPC_R4/Dlss/Fsr3D3D11.cpp:233 dispatches ffxFsr3UpscalerContextDispatch.
- sdk/FSR3/fsr3_dx11.props compiles the DX11 backend and fsr3upscaler components, not frame interpolation or optical flow.
- sdk/FSR3/src/backends/dx11/ffx_dx11.cpp:150 assigns fpSwapChainConfigureFrameGeneration a lambda returning FFX_OK without doing anything. It is not frame-generation support.
- A targeted search of src/Layers and src/xrEngine found no D3D12CreateDevice, D3D11On12CreateDevice, slInit or slReflexSleep integration.
- src/Layers/xrRenderPC_R4/r4_rendertarget_phase_combine.cpp:601,672 draws PP UI before later optical/postprocessing work; lines 742 onward composite PiP, and line 1017 renders scope VSLR. Simply capturing the final backbuffer cannot supply a clean pre-UI scene or correct optical motion/depth.
- sdk/FSR3/include/FidelityFX/host/backends/dx12/ffx_dx12.h has DX12 swapchain interfaces, but headers do not establish a working DX12 engine backend.

## Official documentation checked

- NVIDIA Streamline DLSS-G guide, version 2.14.16: https://github.com/NVIDIA-RTX/Streamline/blob/main/docs/ProgrammingGuideDLSS_G.md
- NVIDIA requirements: https://developer.nvidia.com/rtx/streamline/get-started
- AMD FSR 3.1 integration: https://gpuopen.com/manuals/fidelityfx_sdk/techniques/super-resolution-interpolation/
- Microsoft D3D11On12: https://learn.microsoft.com/en-us/windows/win32/direct3d12/direct3d-11-on-12

NVIDIA's current DLSS-G guide describes D3D12/Vulkan integration, input lifetimes and Reflex. NVIDIA's requirements page explicitly requires DX12 for its DirectX frame-generation path. AMD supplies DX12/Vulkan interpolation swapchains. Neither is a drop-in native DX11 FG backend. The GPU model does not remove this API mismatch.

## Original next options (the shared-resource option is now prototyped above)

1. Investigate a DX12 presentation bridge while retaining DX11 rendering: shared color, depth and motion resources on the same adapter, explicit cross-API synchronization, DX12 swapchain ownership, resize/device-loss handling, and a measured copy/synchronization budget. Prove ordinary ungenerated presentation first. SDK compatibility of this bridge is unverified.
2. Alternatively prototype D3D11On12: replace device/swapchain initialization, acquire/release wrapped backbuffers and flush at ownership transitions. Validate all existing NGX DX11, render-target and shader behavior before adding Streamline. This is a graphics architecture experiment, not an established compatible switch.
3. A native DX12 renderer port is a larger alternative. A custom DX11 optical-flow/interpolation backend also needs an algorithm, pacing, resource lifetime and UI composition implementation; it is not supplied by the current FSR DX11 integration.

After a working graphics bridge, integrate actual capability detection, Reflex and frame generation disabled by default. Keep frame IDs tied to presented main frames, never hidden SVP passes. Separate pre-UI color and UI alpha/color, preserve PiP optical depth/motion or conservatively disable FG while scopes are active until validated. Reset on camera/viewport changes; disable during menus/loading, resize and device loss. Check generated-frame cadence and image quality independently of simulation FPS.

## Initial investigation validation and limits (before prototype)

Performed: local source audit, clean baseline check, remote branch/tag SHA verification, local GPU query and official-document review. No runtime renderer changes, compilation, shader execution or gameplay test occurred. No performance or image-quality claim is made. Installed game/mod files were neither read nor changed. No public release or website update.

## Prototype 1 validation result — 2026-10-18

Built commit 89a4fd9c2ccacb7b108f78ea4c456b1b0e94a930, GitHub Actions run 39099172239. DX11 AVX executable, temporal_test, pda_depth_test, sampler_test and camera_mv_test passed; matching DB0 and complete package produced. Initial standalone compile found a missing UINT64_MAX declaration, fixed before this successful build.

Downloaded artifact 12126162048 to C:/GAMMA/sqa-previews/personal-frame-generation-bridge/Squared Away Personal DX12 Bridge Test 1 DX11 AVX.zip. SHA256 ADF8A137CA0D3F0692A90497AC8616B87D21F5F115828DA736D7D558F77CB59D matched GitHub's digest; all 13 build-manifest files verified.

Local hardware probe exited 0: 60 synchronized copy/present calls each at 320x240, 352x272 and 384x304; two resizes and cleanup completed. Log: C:/GAMMA/sqa-previews/personal-frame-generation-bridge/bridge-probe.log. Hidden-window API success is not proof of visible pixels or gameplay. The probe selects the default hardware adapter; it does not log adapter identity. RTX 5080 presence was established separately.

No in-game test was run. Bodycam/PiP, HUD, PDA, screenshots, upscaler image quality, window-mode transitions and frame-time impact still need Fomok's A/B testing. Frame generation remains unimplemented. Installed files were not read or modified.

Upstream comparison during build: Brodrigz release 0.6/tag 1.0.2 includes an experimental bridge and FG integrations. D3D12PresentBridge.cpp and tools/dlss/DX12_BRIDGE.txt were inspected as references after this prototype was written; no code merged. Its shared-resource mechanism is similar and should be evaluated for the next FG stage, with a separate Bodycam/PiP audit.

## Verified personal FG package
Built source a8ed90397aa0509ed58284d8422274e31c046a37; GitHub Actions run 39101862415 success; artifact 12127236624. ZIP SHA256 44BD25EAE0C9B539AD725CBA28FE1455249014D3C5077DB570EAC220DB7F5B34. All 25 manifest entries and six NVIDIA runtime signatures verified locally. Full DX11 AVX build and existing checks passed. Output: C:/GAMMA/sqa-previews/personal-frame-generation/Squared Away Personal DLSS FG Test 1 DX11 AVX.zip. Local evidence: validation.json, package-verification.json, runtime-probe.log, fg-inputs-test.log in the same directory. No actual generated-frame/gameplay result yet. This documentation update does not change the built engine source.