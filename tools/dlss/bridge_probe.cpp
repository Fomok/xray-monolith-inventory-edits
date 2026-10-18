#define NOMINMAX
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include "../../src/Layers/xrRenderDX10/dx12PresentationBridge.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "user32.lib")

int main(int argc, char** argv)
{
    using Microsoft::WRL::ComPtr;
    auto check = [](HRESULT hr, const char* step) {
        if (FAILED(hr)) { printf("FAIL %s 0x%08lx\n", step, hr); ExitProcess(1); }
    };
    WNDCLASSW wc = {};
    wc.lpfnWndProc = DefWindowProcW; wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"SqaBridgeProbe";
    RegisterClassW(&wc);
    HWND window = CreateWindowW(wc.lpszClassName, L"SQA bridge probe", WS_OVERLAPPEDWINDOW,
        0, 0, 320, 240, nullptr, nullptr, wc.hInstance, nullptr);
    if (!window) return 2; // Hidden window: no game installation involved.
    {
        ComPtr<ID3D11Device> raw;
        ComPtr<ID3D11DeviceContext> rawContext;
        D3D_FEATURE_LEVEL level;
        check(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION,
            &raw, &level, &rawContext), "D3D11 device");
        ComPtr<ID3D11Device1> d11; ComPtr<ID3D11DeviceContext1> c11;
        check(raw.As(&d11), "device1"); check(rawContext.As(&c11), "context1");
        ComPtr<IDXGIFactory2> factory;
        check(CreateDXGIFactory1(IID_PPV_ARGS(&factory)), "factory");
        Dx12PresentationBridge bridge;
        ComPtr<IDXGISwapChain1> swapchain;
        DXGI_SWAP_CHAIN_DESC1 desc = {};
        desc.Width=320; desc.Height=240; desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count=1; desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount=2; desc.SwapEffect=DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.AlphaMode=DXGI_ALPHA_MODE_IGNORE;
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreen = {}; fullscreen.Windowed=TRUE;
        const bool fg = argc>1 && strcmp(argv[1],"--streamline")==0;
        check(bridge.Initialize(d11.Get(), c11.Get(), factory.Get(), window, desc, fullscreen, &swapchain, fg,
            [](const char* message){puts(message);}), "initialize");
        for (unsigned pass=0; pass<8; ++pass)
        {
            if (pass) {
                check(bridge.Suspend(), "drain before resize");
                c11->ClearState(); c11->Flush();
                desc.Width += 32; desc.Height += 32;
                check(swapchain->ResizeBuffers(2,desc.Width,desc.Height,desc.Format,0), "resize");
            }
            ComPtr<ID3D11Texture2D> target;
            check(bridge.Target(d11.Get(),desc.Width,desc.Height,desc.Format,&target), "shared target");
            ComPtr<ID3D11RenderTargetView> rtv;
            check(d11->CreateRenderTargetView(target.Get(),nullptr,&rtv), "RTV");
            for (unsigned frame=0; frame<120; ++frame) {
                check(bridge.BeginFrame(0), "Reflex begin"); bridge.EndSimulation();
                const float color[4] = { frame/120.f, pass/8.f, 0.25f, 1.f };
                c11->ClearRenderTargetView(rtv.Get(),color);
                check(bridge.Present(swapchain.Get(),0,0), "copy and present");
            }
            check(bridge.Suspend(), "drain before readback");
            D3D11_TEXTURE2D_DESC readDesc{}; target->GetDesc(&readDesc);
            readDesc.Usage=D3D11_USAGE_STAGING; readDesc.BindFlags=0;
            readDesc.CPUAccessFlags=D3D11_CPU_ACCESS_READ; readDesc.MiscFlags=0;
            ComPtr<ID3D11Texture2D> readback;
            check(d11->CreateTexture2D(&readDesc,nullptr,&readback), "readback texture");
            c11->CopyResource(readback.Get(),target.Get());
            D3D11_MAPPED_SUBRESOURCE mapped{};
            check(c11->Map(readback.Get(),0,D3D11_MAP_READ,0,&mapped), "readback map");
            const auto* pixel=static_cast<const unsigned char*>(mapped.pData);
            const bool correct=abs(int(pixel[0])-int(255.f*119.f/120.f+.5f))<=1 &&
                abs(int(pixel[1])-int(255.f*pass/8.f+.5f))<=1 && abs(int(pixel[2])-64)<=1 && pixel[3]==255;
            c11->Unmap(readback.Get(),0);
            if(!correct){puts("FAIL shared target pixel after retirement");return 3;}
            printf("PASS %ux%u: 120 presentations and shared-target readback\n",desc.Width,desc.Height);
        }
        c11->ClearState(); c11->Flush();
        bridge.ShutdownBeforeSwapchainRelease();
        swapchain.Reset();
    }
    DestroyWindow(window);
    puts("PASS bridge initialization, 960 presentations, seven resizes, eight shared-target readbacks and cleanup. Not a gameplay or displayed-image test.");
    return 0;
}
