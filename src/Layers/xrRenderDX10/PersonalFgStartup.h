#pragma once
#include "dx12PresentationBridge.h"

// Shared by normal game startup and the isolated startup probe. Capability setup
// does not enable generation: the existing scene/settings guards own that choice.
inline HRESULT CreatePersonalPresentation(ID3D11Device1* device, ID3D11DeviceContext1* context,
    IDXGIFactory2* factory, HWND window, const DXGI_SWAP_CHAIN_DESC1& desc,
    const DXGI_SWAP_CHAIN_FULLSCREEN_DESC& fullscreen, IDXGISwapChain1** swapchain,
    Dx12PresentationBridge*& bridge, bool allowFg, StreamlineIntegration::Logger logger)
{
    bridge=nullptr;
    *swapchain=nullptr;
    if(allowFg)
    {
        bridge=new Dx12PresentationBridge();
        const HRESULT result=bridge->Initialize(device,context,factory,window,desc,fullscreen,swapchain,true,logger);
        if(SUCCEEDED(result))
        {
            if(logger)logger("[Personal FG] Automatic startup ready; generation follows saved Video settings (default Off). No launch environment required.");
            return result;
        }
        if(logger)
        {
            char message[160]{};
            snprintf(message,sizeof(message),"[Personal FG] Automatic startup unavailable (0x%08x); falling back to native DX11. Saved FG/PiP choices retained.",unsigned(result));
            logger(message);
        }
        // Destroy the attempted proxy before creating a native swapchain for the
        // same HWND. Shutdown/release ordering matches the existing failure path.
        bridge->ShutdownBeforeSwapchainRelease();
        if(*swapchain){(*swapchain)->Release();*swapchain=nullptr;}
        delete bridge;bridge=nullptr;
    }
    return factory->CreateSwapChainForHwnd(device,window,&desc,&fullscreen,nullptr,swapchain);
}
