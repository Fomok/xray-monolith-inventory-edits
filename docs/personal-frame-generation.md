# Personal Bodycam + DLSS frame generation investigation

Checked 2026-10-18. Status: blocked on graphics/presentation architecture; frame generation is NOT implemented. No toggle, binaries or test package are supplied.

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

## Concrete next options

1. Investigate a DX12 presentation bridge while retaining DX11 rendering: shared color, depth and motion resources on the same adapter, explicit cross-API synchronization, DX12 swapchain ownership, resize/device-loss handling, and a measured copy/synchronization budget. Prove ordinary ungenerated presentation first. SDK compatibility of this bridge is unverified.
2. Alternatively prototype D3D11On12: replace device/swapchain initialization, acquire/release wrapped backbuffers and flush at ownership transitions. Validate all existing NGX DX11, render-target and shader behavior before adding Streamline. This is a graphics architecture experiment, not an established compatible switch.
3. A native DX12 renderer port is a larger alternative. A custom DX11 optical-flow/interpolation backend also needs an algorithm, pacing, resource lifetime and UI composition implementation; it is not supplied by the current FSR DX11 integration.

After a working graphics bridge, integrate actual capability detection, Reflex and frame generation disabled by default. Keep frame IDs tied to presented main frames, never hidden SVP passes. Separate pre-UI color and UI alpha/color, preserve PiP optical depth/motion or conservatively disable FG while scopes are active until validated. Reset on camera/viewport changes; disable during menus/loading, resize and device loss. Check generated-frame cadence and image quality independently of simulation FPS.

## Validation and limits

Performed: local source audit, clean baseline check, remote branch/tag SHA verification, local GPU query and official-document review. No runtime renderer changes, compilation, shader execution or gameplay test occurred. No performance or image-quality claim is made. Installed game/mod files were neither read nor changed. No public release or website update.
