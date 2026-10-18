#pragma once

// Personal prototype: native D3D11 rendering, D3D12 copy/presentation only.
// No interpolation. Both APIs use the adapter owning the actual D3D11 device.
#include <d3d11_4.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

class Dx12PresentationBridge
{
    template<class T> using Ptr = Microsoft::WRL::ComPtr<T>;
    HMODULE runtime = nullptr;
    HANDLE completion = nullptr;
    Ptr<ID3D12Device> device;
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

    HRESULT WaitComplete()
    {
        if (!serial || !fence) return S_OK;
        const UINT64 done = fence->GetCompletedValue();
        if (done == ~UINT64(0)) return DXGI_ERROR_DEVICE_REMOVED;
        if (done >= serial) return S_OK;
        HRESULT hr = fence->SetEventOnCompletion(serial, completion);
        if (FAILED(hr)) return hr;
        DWORD wait = WaitForSingleObject(completion, 5000);
        if (wait != WAIT_OBJECT_0) return wait == WAIT_TIMEOUT ? DXGI_ERROR_DEVICE_HUNG : HRESULT_FROM_WIN32(GetLastError());
        return device->GetDeviceRemovedReason();
    }

public:
    ~Dx12PresentationBridge()
    {
        WaitComplete();
        target.Reset(); shared.Reset(); context.Reset(); fence11.Reset();
        fence.Reset(); commands.Reset(); allocator.Reset(); queue.Reset(); device.Reset();
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
        IDXGISwapChain1** swapchain)
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
        runtime = LoadLibraryExW(L"d3d12.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (!runtime) return HRESULT_FROM_WIN32(GetLastError());
        auto create = reinterpret_cast<PFN_D3D12_CREATE_DEVICE>(GetProcAddress(runtime, "D3D12CreateDevice"));
        if (!create) return E_NOINTERFACE;
        hr = create(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.GetAddressOf()));
        if (FAILED(hr)) return hr;
        D3D12_COMMAND_QUEUE_DESC q = {};
        q.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        hr = device->CreateCommandQueue(&q, IID_PPV_ARGS(queue.GetAddressOf()));
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
        return factory->CreateSwapChainForHwnd(queue.Get(), window, &desc, &fullscreen, nullptr, swapchain);
    }

    HRESULT Present(IDXGISwapChain1* swapchain, UINT interval, UINT flags)
    {
        HRESULT hr = WaitComplete();
        if (FAILED(hr)) return hr;
        // Serialize the two APIs. The CPU completion wait also protects the next
        // D3D11 frame, resize and allocator reuse. Optimize only after gameplay proof.
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
        hr = WaitComplete();
        if (FAILED(hr)) return hr;
        return swapchain->Present(interval, flags);
    }
};
