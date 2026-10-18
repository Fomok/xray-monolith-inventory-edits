#define NOMINMAX
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include "../../src/Layers/xrRenderDX10/dx12PresentationBridge.h"
#include "../../src/Layers/xrRenderPC_R4/Dlss/DlssD3D11.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "user32.lib")

int main(int argc, char** argv)
{
    bool fg=false, ngx=false, generate=false;
    for(int i=1;i<argc;++i) { fg |= strcmp(argv[i],"--streamline")==0; ngx |= strcmp(argv[i],"--ngx")==0; generate |= strcmp(argv[i],"--generate")==0; }
    if(generate && (!fg || !ngx)) { puts("--generate requires --streamline --ngx"); return 7; }
    unsigned generatedPresents=0;
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
    if (!window) return 2;
    // An occluded/hidden swapchain can legitimately present only one frame even
    // when DLSS-G accepts the tags. Active-generation validation needs a visible
    // presentation surface; transport-only modes remain hidden.
    if(generate) { ShowWindow(window,SW_SHOWNORMAL);UpdateWindow(window);SetForegroundWindow(window); }
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
        if(ngx) { desc.Width=1280; desc.Height=720; }
        check(bridge.Initialize(d11.Get(), c11.Get(), factory.Get(), window, desc, fullscreen, &swapchain, fg,
            [](const char* message){puts(message);}), "initialize");
        dlss::D3D11Backend upscaler;
        if(ngx && !upscaler.Initialize(raw.Get(),L".",[](const char* message,unsigned result){printf("NGX %s 0x%08x\n",message,result);})) return 4;
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
            ComPtr<ID3D11Texture2D> colorInput,depthInput,motionInput;
            ComPtr<ID3D11RenderTargetView> colorRtv,depthRtv,motionRtv;
            if(ngx)
            {
                // Recreate NGX on each pass to exercise backend/resource lifetime
                // with the presentation bridge still alive. Synthetic inputs only.
                if(!upscaler.Create(dlss::Quality::DLAA,{desc.Width,desc.Height},{desc.Width,desc.Height},false)) return 5;
                auto input=[&](DXGI_FORMAT format,ComPtr<ID3D11Texture2D>& texture,ComPtr<ID3D11RenderTargetView>& view){
                    D3D11_TEXTURE2D_DESC d{}; d.Width=desc.Width;d.Height=desc.Height;
                    d.MipLevels=d.ArraySize=d.SampleDesc.Count=1;d.Format=format;
                    d.BindFlags=D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;
                    check(d11->CreateTexture2D(&d,nullptr,&texture),"NGX input");
                    check(d11->CreateRenderTargetView(texture.Get(),nullptr,&view),"NGX input RTV");
                };
                input(DXGI_FORMAT_R16G16B16A16_FLOAT,colorInput,colorRtv);
                input(DXGI_FORMAT_R32_FLOAT,depthInput,depthRtv);
                input(DXGI_FORMAT_R16G16_FLOAT,motionInput,motionRtv);
                const float depthColor[]={.5f,0,0,0},zero[]={0,0,0,0};
                c11->ClearRenderTargetView(depthRtv.Get(),depthColor);c11->ClearRenderTargetView(motionRtv.Get(),zero);
            }
            for (unsigned frame=0; frame<120; ++frame) {
                MSG message{};
                while(PeekMessageW(&message,nullptr,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
                check(bridge.BeginFrame(0), "Reflex begin"); bridge.EndSimulation();
                const float color[4] = { frame/120.f, pass/8.f, 0.25f, 1.f };
                if(ngx)
                {
                    c11->ClearRenderTargetView(colorRtv.Get(),color);
                    dlss::D3D11Backend::Frame f{};
                    f.color=colorInput.Get();f.depth=depthInput.Get();f.motion=motionInput.Get();
                    f.jitter={0,0};f.deltaMilliseconds=16.666f;f.frame=pass*120+frame;f.cameraCut=frame==0;
                    if(!upscaler.Evaluate(f)) return 6;
                }
                c11->ClearRenderTargetView(rtv.Get(),color);
                if(generate)
                {
                    sl::Constants c{};
                    sl::float4x4 identity{};
                    identity[0]={1,0,0,0};identity[1]={0,1,0,0};identity[2]={0,0,1,0};identity[3]={0,0,0,1};
                    c.cameraViewToClip=c.clipToCameraView=c.clipToLensClip=c.clipToPrevClip=c.prevClipToClip=identity;
                    c.jitterOffset=c.cameraPinholeOffset={0,0};c.mvecScale={1,1};
                    c.cameraPos={0,0,0};c.cameraUp={0,1,0};c.cameraRight={1,0,0};c.cameraFwd={0,0,1};
                    c.cameraNear=.1f;c.cameraFar=1.f;c.cameraFOV=1.f;c.cameraAspectRatio=float(desc.Width)/desc.Height;
                    c.depthInverted=c.motionVectors3D=c.motionVectorsJittered=sl::eFalse;
                    c.cameraMotionIncluded=c.orthographicProjection=sl::eTrue;c.reset=frame==0?sl::eTrue:sl::eFalse;
                    const HRESULT captured=bridge.CaptureScene(depthInput.Get(),motionInput.Get(),c);
                    if(captured!=S_OK){printf("FAIL FG capture 0x%08lx\n",captured);return 8;}
                    bridge.CaptureHudless();
                    const float overlay[]={1,1,1,1};D3D11_RECT rectangle{20,20,40,40};
                    c11->ClearView(rtv.Get(),overlay,&rectangle,1);
                }
                check(bridge.Present(swapchain.Get(),0,0), "copy and present");
                if(generate && bridge.GenerationActive() && bridge.PresentedFrames()>=2) ++generatedPresents;
            }
            check(bridge.Suspend(), "drain before readback");
            if(ngx) upscaler.ReleaseFeature();
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
        upscaler.Shutdown();
        bridge.ShutdownBeforeSwapchainRelease();
        swapchain.Reset();
    }
    DestroyWindow(window);
    FILE* report=nullptr;fopen_s(&report,"bridge-probe-result.txt","w");
    if(report){fprintf(report,"Completed=960 Resizes=7 Readbacks=8 NGX=%u GenerateRequested=%u GeneratedPresents=%u\n",unsigned(ngx),unsigned(generate),generatedPresents);fclose(report);}
    if(generate && generatedPresents<900){puts("FAIL fewer than 900 confirmed generated presentations");return 9;}
    puts("PASS bridge initialization, 960 presentations, seven resizes, eight shared-target readbacks and cleanup. Not a gameplay or displayed-image test.");
    return 0;
}
