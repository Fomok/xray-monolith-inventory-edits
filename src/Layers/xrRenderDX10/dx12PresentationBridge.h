#pragma once

// Personal bridge: native D3D11 rendering, optional DLSS-G on D3D12 presentation.
#include <d3d11_4.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <cstdio>
#include "StreamlineIntegration.h"
#include "PersonalFgInputs.h"

class Dx12PresentationBridge
{
    template<class T> using Ptr = Microsoft::WRL::ComPtr<T>;
    HMODULE runtime = nullptr;
    HANDLE completion = nullptr;
    Ptr<ID3D12Device> device;
    Ptr<ID3D12Device> proxyDevice;
    Ptr<ID3D11Device1> renderDevice;
    StreamlineIntegration streamline;
    PersonalFgInputs inputs;
    sl::Constants frameConstants{};
    bool fgReady = false, fgFailed = false;
    StreamlineIntegration::Logger logger = nullptr;
    Ptr<ID3D12CommandQueue> queue;
    Ptr<ID3D12CommandAllocator> allocator;
    Ptr<ID3D12GraphicsCommandList> commands;
    Ptr<ID3D12Fence> fence;
    Ptr<ID3D11Fence> fence11;
    Ptr<ID3D11DeviceContext4> context;
    Ptr<ID3D12Resource> shared;
    Ptr<ID3D11Texture2D> target;
    UINT64 serial = 0;
    UINT width = 0, height = 0;
    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    double waitMs = 0, reflexMs = 0, presentMs = 0;
    UINT timingPresents = 0, timingLimitUs = 0;
    static double NowMs()
    {
        static const double frequency = [] { LARGE_INTEGER f; QueryPerformanceFrequency(&f); return double(f.QuadPart); }();
        LARGE_INTEGER t; QueryPerformanceCounter(&t); return double(t.QuadPart) * 1000.0 / frequency;
    }

    HRESULT WaitComplete()
    {
        if (!serial || !fence) return S_OK;
        const UINT64 done = fence->GetCompletedValue();
        if (done == ~UINT64(0)) return DXGI_ERROR_DEVICE_REMOVED;
        if (done >= serial) return S_OK;
        HRESULT hr = fence->SetEventOnCompletion(serial, completion);
        if (FAILED(hr)) return hr;
        const double start = NowMs();
        DWORD wait = WaitForSingleObject(completion, 5000);
        waitMs += NowMs() - start;
        if (wait != WAIT_OBJECT_0)
        {
            const HRESULT removed = device->GetDeviceRemovedReason();
            if (logger)
            {
                char message[320];
                std::snprintf(message, sizeof(message), "! [DX12 bridge] retirement wait=%lu expected=%llu completed=%llu DX12 removed=0x%08x; timeout is not itself proof of device removal",
                    wait, serial, fence->GetCompletedValue(), unsigned(removed));
                logger(message);
            }
            if (FAILED(removed)) return removed;
            return HRESULT_FROM_WIN32(wait == WAIT_TIMEOUT ? ERROR_TIMEOUT : GetLastError());
        }
        return device->GetDeviceRemovedReason();
    }

public:
    bool GenerationReady() const { return fgReady && !fgFailed; }
    bool ReflexReady() const { return fgReady; }
    bool GenerationActive() const { return fgReady && streamline.IsEnabled(); }
    bool NeedsHudless() const { return inputs.scene && !inputs.hudless; }
    UINT PresentedFrames() const { return streamline.PresentedFrames(); }
    void DiscardInputs() { inputs.Invalidate(); }
    HRESULT BeginFrame(UINT limitUs)
    {
        inputs.Invalidate();
        timingLimitUs = limitUs;
        const double start = NowMs();
        const bool ready = !fgReady || streamline.BeginFrame(limitUs);
        reflexMs += NowMs() - start;
        if (!ready) return E_FAIL;
        return S_OK;
    }
    void EndSimulation()
    {
        if (!fgReady) return;
        streamline.Marker(sl::PCLMarker::eSimulationEnd);
        streamline.Marker(sl::PCLMarker::eRenderSubmitStart);
    }
    HRESULT Suspend()
    {
        HRESULT hr=WaitComplete(); if(FAILED(hr))return hr;
        inputs.Invalidate();
        if(fgReady && (!streamline.SetEnabled(false) || !streamline.ClearTags()))return E_FAIL;
        return S_OK;
    }
    HRESULT FailGeneration()
    {
        HRESULT hr=WaitComplete(); if(FAILED(hr))return hr;
        fgFailed=true; inputs.Invalidate();
        if(logger)logger("[DLSS FG] Disabled after input/runtime failure; restart game to retry.");
        return !fgReady || streamline.DisableAfterFailure() ? S_OK : E_FAIL;
    }
    HRESULT CaptureScene(ID3D11Texture2D* depth, ID3D11Texture2D* motion, const sl::Constants& constants)
    {
        if(!GenerationReady() || !streamline.HasFrame() || !streamline.CanGenerate(width,height))return S_FALSE;
        if(!depth || !motion)return E_INVALIDARG;
        D3D11_TEXTURE2D_DESC d{},m{}; depth->GetDesc(&d); motion->GetDesc(&m);
        if(d.Width!=m.Width || d.Height!=m.Height || d.SampleDesc.Count!=1 || m.SampleDesc.Count!=1)return E_INVALIDARG;
        if(!inputs.Matches(d.Width,d.Height,width,height))
        {
            HRESULT hr=Suspend(); if(FAILED(hr))return hr;
            hr=inputs.Resize(device.Get(),renderDevice.Get(),d.Width,d.Height,width,height); if(FAILED(hr))return hr;
        }
        frameConstants=constants;
        return inputs.Capture(renderDevice.Get(),context.Get(),depth,motion);
    }
    void CaptureHudless()
    {
        if(NeedsHudless()) { context->CopyResource(inputs.render[2].Get(),target.Get()); inputs.hudless=true; }
    }
    void ShutdownBeforeSwapchainRelease()
    {
        WaitComplete();
        streamline.Shutdown(); fgReady=false;
    }
    ~Dx12PresentationBridge()
    {
        ShutdownBeforeSwapchainRelease();
        inputs=PersonalFgInputs{}; renderDevice.Reset();
        target.Reset(); shared.Reset(); context.Reset(); fence11.Reset();
        fence.Reset(); commands.Reset(); allocator.Reset(); queue.Reset(); proxyDevice.Reset(); device.Reset();
        if (completion) CloseHandle(completion);
        if (runtime) FreeLibrary(runtime);
    }

    HRESULT Target(ID3D11Device1* d11, UINT w, UINT h, DXGI_FORMAT fmt, ID3D11Texture2D** result)
    {
        *result = nullptr;
        if (!target || w != width || h != height || fmt != format)
        {
            HRESULT hr = WaitComplete();
            if (FAILED(hr)) return hr;
            D3D12_HEAP_PROPERTIES heap = {};
            heap.Type = D3D12_HEAP_TYPE_DEFAULT;
            heap.CreationNodeMask = heap.VisibleNodeMask = 1;
            D3D12_RESOURCE_DESC desc = {};
            desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            desc.Width = w; desc.Height = h; desc.DepthOrArraySize = 1; desc.MipLevels = 1;
            desc.Format = fmt; desc.SampleDesc.Count = 1;
            desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
            Ptr<ID3D12Resource> next;
            hr = device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_SHARED, &desc,
                D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(next.GetAddressOf()));
            if (FAILED(hr)) return hr;
            HANDLE handle = nullptr;
            hr = device->CreateSharedHandle(next.Get(), nullptr, GENERIC_ALL, nullptr, &handle);
            if (FAILED(hr)) return hr;
            Ptr<ID3D11Texture2D> next11;
            hr = d11->OpenSharedResource1(handle, IID_PPV_ARGS(next11.GetAddressOf()));
            CloseHandle(handle);
            if (FAILED(hr)) return hr;
            // Validate renderability before committing a swapchain during startup.
            Ptr<ID3D11RenderTargetView> view;
            hr = d11->CreateRenderTargetView(next11.Get(), nullptr, view.GetAddressOf());
            if (FAILED(hr)) return hr;
            shared = next; target = next11; width = w; height = h; format = fmt;
        }
        return target.CopyTo(result);
    }

    HRESULT Initialize(ID3D11Device1* d11, ID3D11DeviceContext1* c11, IDXGIFactory2* factory,
        HWND window, const DXGI_SWAP_CHAIN_DESC1& desc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC& fullscreen,
        IDXGISwapChain1** swapchain, bool enableFg = false, StreamlineIntegration::Logger log = nullptr)
    {
        // HDR and exclusive mode remain on the established native path for prototype 1.
        if (!fullscreen.Windowed || desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM || desc.SampleDesc.Count != 1)
            return DXGI_ERROR_UNSUPPORTED;
        Ptr<ID3D11Device5> device11;
        HRESULT hr = d11->QueryInterface(IID_PPV_ARGS(device11.GetAddressOf()));
        if (FAILED(hr)) return hr;
        hr = c11->QueryInterface(IID_PPV_ARGS(context.GetAddressOf()));
        if (FAILED(hr)) return hr;
        Ptr<IDXGIDevice> dxgi;
        Ptr<IDXGIAdapter> adapter;
        hr = d11->QueryInterface(IID_PPV_ARGS(dxgi.GetAddressOf()));
        if (FAILED(hr)) return hr;
        hr = dxgi->GetAdapter(adapter.GetAddressOf());
        if (FAILED(hr)) return hr;
        logger=log;
        renderDevice=d11;
        if(enableFg)
        {
            DXGI_ADAPTER_DESC adapterDesc{};
            hr=adapter->GetDesc(&adapterDesc); if(FAILED(hr))return hr;
            wchar_t executable[32768]{};
            DWORD length=GetModuleFileNameW(nullptr,executable,32768);
            if(!length || length>=32768)return E_FAIL;
            std::wstring directory(executable);
            directory.resize(directory.find_last_of(L"\\/")); directory+=L"\\streamline";
            if(!streamline.Initialize(directory.c_str(),logger) || !streamline.CheckAdapter(adapterDesc.AdapterLuid))return E_FAIL;
        }
        runtime = LoadLibraryExW(L"d3d12.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (!runtime) return HRESULT_FROM_WIN32(GetLastError());
        auto create = reinterpret_cast<PFN_D3D12_CREATE_DEVICE>(GetProcAddress(runtime, "D3D12CreateDevice"));
        if (!create) return E_NOINTERFACE;
        hr = create(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.GetAddressOf()));
        if (FAILED(hr)) return hr;
        if(enableFg)
        {
            if(!streamline.SetDevice(device.Get()))return E_FAIL;
            proxyDevice=device;
            auto* proxy=proxyDevice.Detach();
            bool upgraded=streamline.Upgrade(reinterpret_cast<void**>(&proxy));
            proxyDevice.Attach(proxy); if(!upgraded)return E_FAIL;
            fgReady=true;
        }
        D3D12_COMMAND_QUEUE_DESC q = {};
        q.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        hr = (proxyDevice ? proxyDevice.Get() : device.Get())->CreateCommandQueue(&q, IID_PPV_ARGS(queue.GetAddressOf()));
        if (FAILED(hr)) return hr;
        hr = device->CreateCommandAllocator(q.Type, IID_PPV_ARGS(allocator.GetAddressOf()));
        if (FAILED(hr)) return hr;
        hr = device->CreateCommandList(0, q.Type, allocator.Get(), nullptr, IID_PPV_ARGS(commands.GetAddressOf()));
        if (FAILED(hr)) return hr;
        hr = commands->Close();
        if (FAILED(hr)) return hr;
        hr = device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(fence.GetAddressOf()));
        if (FAILED(hr)) return hr;
        HANDLE handle = nullptr;
        hr = device->CreateSharedHandle(fence.Get(), nullptr, GENERIC_ALL, nullptr, &handle);
        if (FAILED(hr)) return hr;
        hr = device11->OpenSharedFence(handle, IID_PPV_ARGS(fence11.GetAddressOf()));
        CloseHandle(handle);
        if (FAILED(hr)) return hr;
        completion = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (!completion) return HRESULT_FROM_WIN32(GetLastError());
        Ptr<ID3D11Texture2D> test;
        hr = Target(d11, desc.Width, desc.Height, desc.Format, test.GetAddressOf());
        if (FAILED(hr)) return hr;
        // All interop resources validated before acquiring the window's swapchain.
        Ptr<IDXGIFactory2> presentationFactory=factory;
        if(enableFg)
        {
            auto* proxy=presentationFactory.Detach();
            bool upgraded=streamline.Upgrade(reinterpret_cast<void**>(&proxy));
            presentationFactory.Attach(proxy); if(!upgraded)return E_FAIL;
        }
        return presentationFactory->CreateSwapChainForHwnd(queue.Get(), window, &desc, &fullscreen, nullptr, swapchain);
    }

    HRESULT Present(IDXGISwapChain1* swapchain, UINT interval, UINT flags)
    {
        HRESULT hr = WaitComplete();
        if (FAILED(hr)) return hr;
        if(fgReady)
        {
            if(!streamline.HasFrame()) { if(!streamline.BeginFrame(0))return E_FAIL; EndSimulation(); }
            bool generate=GenerationReady() && inputs.scene && inputs.hudless &&
                streamline.CanGenerate(width,height) && (interval==0 || streamline.SupportsVSync());
            if(generate)
            {
                hr=inputs.ProtectUi(renderDevice.Get(),context.Get(),target.Get());
                if(FAILED(hr)) { hr=FailGeneration(); if(FAILED(hr))return hr; generate=false; }
            }
            if(!streamline.SetEnabled(generate))return E_FAIL;
            bool tagged=generate ? streamline.Tag(frameConstants,inputs.gpu[0].Get(),inputs.gpu[1].Get(),inputs.gpu[2].Get(),inputs.gpu[3].Get()) : streamline.ClearTags();
            if(!tagged) { hr=FailGeneration(); if(FAILED(hr))return hr; }
        }
        // D3D12 consumes this frame only after D3D11 finishes writing it.
        // WaitComplete above retires the previous allocator/tag set, not this frame.
        hr = context->Signal(fence11.Get(), ++serial);
        if (FAILED(hr)) return hr;
        context->Flush();
        hr = queue->Wait(fence.Get(), serial);
        if (FAILED(hr)) return hr;
        Ptr<IDXGISwapChain3> chain3;
        hr = swapchain->QueryInterface(IID_PPV_ARGS(chain3.GetAddressOf()));
        if (FAILED(hr)) return hr;
        Ptr<ID3D12Resource> backbuffer;
        hr = swapchain->GetBuffer(chain3->GetCurrentBackBufferIndex(), IID_PPV_ARGS(backbuffer.GetAddressOf()));
        if (FAILED(hr)) return hr;
        hr = allocator->Reset();
        if (FAILED(hr)) return hr;
        hr = commands->Reset(allocator.Get(), nullptr);
        if (FAILED(hr)) return hr;
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = backbuffer.Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
        commands->ResourceBarrier(1, &barrier);
        // Simultaneous-access shared texture promotes COMMON -> COPY_SOURCE and
        // decays to COMMON when ExecuteCommandLists finishes.
        commands->CopyResource(backbuffer.Get(), shared.Get());
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        commands->ResourceBarrier(1, &barrier);
        hr = commands->Close();
        if (FAILED(hr)) return hr;
        ID3D12CommandList* lists[] = { commands.Get() };
        queue->ExecuteCommandLists(1, lists);
        hr = queue->Signal(fence.Get(), ++serial);
        if (FAILED(hr)) return hr;
        if(fgReady) { streamline.Marker(sl::PCLMarker::eRenderSubmitEnd); streamline.Marker(sl::PCLMarker::ePresentStart); }
        const double presentStart = NowMs();
        hr=swapchain->Present(interval, flags);
        presentMs += NowMs() - presentStart;
        if(fgReady)
        {
            streamline.Marker(sl::PCLMarker::ePresentEnd);
            if(FAILED(hr))return hr;
            Ptr<ID3D12Fence> consumed;
            UINT64 value=0;
            bool stateOk=streamline.AfterPresent(consumed.GetAddressOf(),value);
            if(consumed && value) { hr=queue->Wait(consumed.Get(),value); if(FAILED(hr))return hr; }
            // eBlockPresentingClientQueue orders all FG input consumption before
            // this retirement fence, including runtimes without a returned fence.
            hr=queue->Signal(fence.Get(),++serial); if(FAILED(hr))return hr;
            if(!stateOk) return FailGeneration();
        }
        if(FAILED(hr))return hr;
        // Enqueue protection for every future D3D11 write, including hidden PiP
        // passes and screenshots. The CPU can build the next frame while this
        // frame completes. FG retirement is still ordered by its completion fence
        // and eBlockPresentingClientQueue; no shared input may be overwritten early.
        hr=context->Wait(fence11.Get(),serial); if(FAILED(hr))return hr;
        context->Flush();
        inputs.Invalidate();
        if (++timingPresents == 240)
        {
            if (logger)
            {
                char message[256]{};
                snprintf(message,sizeof(message),"[FG TIMING] 240 presents: CPU fence wait=%.3f ms/present, Reflex begin=%.3f ms/present, Present=%.3f ms/present, limitUs=%u, FG=%u, syncInterval=%u. Not GPU execution times.",
                    waitMs/240.0,reflexMs/240.0,presentMs/240.0,timingLimitUs,unsigned(GenerationActive()),interval);
                logger(message);
            }
            timingPresents=0; waitMs=reflexMs=presentMs=0;
        }
        return hr;
    }
};
