#pragma once
// Shared-input transport for the personal DLSS-G bridge. Depth/motion conversion
// follows Brodrigz 1.0.2 / OGSR (xrSimpAI); UI protection is specific to this test.
#include <d3d11_4.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#pragma comment(lib, "d3dcompiler.lib")

class PersonalFgInputs
{
    template<class T> using Ptr = Microsoft::WRL::ComPtr<T>;
    Ptr<ID3DDeviceContextState> captureState;
    Ptr<ID3D11ComputeShader> conversion, uiMask;
    Ptr<ID3D11UnorderedAccessView> depthUav, motionUav, uiUav;
    UINT renderWidth = 0, renderHeight = 0, displayWidth = 0, displayHeight = 0;

    HRESULT Shared(ID3D12Device* d12, ID3D11Device1* d11, UINT index, UINT w, UINT h, DXGI_FORMAT format, bool uav)
    {
        D3D12_HEAP_PROPERTIES heap = {};
        heap.Type = D3D12_HEAP_TYPE_DEFAULT; heap.CreationNodeMask = heap.VisibleNodeMask = 1;
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width = w; desc.Height = h; desc.DepthOrArraySize = desc.MipLevels = 1;
        desc.SampleDesc.Count = 1; desc.Format = format;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
        if (uav) desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        HRESULT hr = d12->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_SHARED, &desc,
            D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(gpu[index].ReleaseAndGetAddressOf()));
        if (FAILED(hr)) return hr;
        HANDLE handle = nullptr;
        hr = d12->CreateSharedHandle(gpu[index].Get(), nullptr, GENERIC_ALL, nullptr, &handle);
        if (FAILED(hr)) return hr;
        hr = d11->OpenSharedResource1(handle, IID_PPV_ARGS(render[index].ReleaseAndGetAddressOf()));
        CloseHandle(handle);
        return hr;
    }
    HRESULT State(ID3D11Device1* device)
    {
        if (captureState) return S_OK;
        const D3D_FEATURE_LEVEL level = device->GetFeatureLevel();
        D3D_FEATURE_LEVEL selected;
        return device->CreateDeviceContextState(0, &level, 1, D3D11_SDK_VERSION,
            __uuidof(ID3D11Device), &selected, captureState.GetAddressOf());
    }
    struct Restore
    {
        ID3D11DeviceContext1* context;
        Ptr<ID3DDeviceContextState> previous;
        ~Restore() { context->ClearState(); context->SwapDeviceContextState(previous.Get(), nullptr); }
    };
    HRESULT Shader(ID3D11Device1* device, const char* code, SIZE_T size, Ptr<ID3D11ComputeShader>& shader)
    {
        if (shader) return S_OK;
        Ptr<ID3DBlob> blob, errors;
        HRESULT hr = D3DCompile(code, size, "SQA FG inputs", nullptr, nullptr, "main", "cs_5_0",
            D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, blob.GetAddressOf(), errors.GetAddressOf());
        if (FAILED(hr)) return hr;
        return device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shader.GetAddressOf());
    }
public:
    Ptr<ID3D12Resource> gpu[4]; // depth, motion, HUD-less color, conservative UI alpha
    Ptr<ID3D11Texture2D> render[4];
    bool scene = false, hudless = false;
    bool Matches(UINT rw, UINT rh, UINT dw, UINT dh) const
    { return renderWidth == rw && renderHeight == rh && displayWidth == dw && displayHeight == dh; }
    void Invalidate() { scene = hudless = false; }
    HRESULT Resize(ID3D12Device* d12, ID3D11Device1* d11, UINT rw, UINT rh, UINT dw, UINT dh)
    {
        renderWidth = renderHeight = displayWidth = displayHeight = 0;
        Invalidate(); depthUav.Reset(); motionUav.Reset(); uiUav.Reset();
        HRESULT hr = Shared(d12,d11,0,rw,rh,DXGI_FORMAT_R32_FLOAT,true);
        if (FAILED(hr)) return hr;
        hr = Shared(d12,d11,1,rw,rh,DXGI_FORMAT_R16G16_FLOAT,true);
        if (FAILED(hr)) return hr;
        hr = Shared(d12,d11,2,dw,dh,DXGI_FORMAT_R8G8B8A8_UNORM,false);
        if (FAILED(hr)) return hr;
        hr = Shared(d12,d11,3,dw,dh,DXGI_FORMAT_R8_UNORM,true);
        if (FAILED(hr)) return hr;
        hr = d11->CreateUnorderedAccessView(render[0].Get(),nullptr,depthUav.GetAddressOf());
        if (FAILED(hr)) return hr;
        hr = d11->CreateUnorderedAccessView(render[1].Get(),nullptr,motionUav.GetAddressOf());
        if (FAILED(hr)) return hr;
        hr = d11->CreateUnorderedAccessView(render[3].Get(),nullptr,uiUav.GetAddressOf());
        if (FAILED(hr)) return hr;
        renderWidth=rw; renderHeight=rh; displayWidth=dw; displayHeight=dh;
        return S_OK;
    }
    HRESULT Capture(ID3D11Device1* device, ID3D11DeviceContext1* context, ID3D11Texture2D* depth, ID3D11Texture2D* motion)
    {
        static const char code[] = R"(
Texture2D<float> depthIn:register(t0); Texture2D<float4> motionIn:register(t1);
RWTexture2D<float> depthOut:register(u0); RWTexture2D<float2> motionOut:register(u1);
[numthreads(8,8,1)] void main(uint3 p:SV_DispatchThreadID) {
 uint w,h; depthOut.GetDimensions(w,h); if(p.x>=w||p.y>=h)return;
 depthOut[p.xy]=depthIn.Load(int3(p.xy,0)); motionOut[p.xy]=motionIn.Load(int3(p.xy,0)).xy;
})";
        HRESULT hr = Shader(device,code,sizeof(code)-1,conversion);
        if (FAILED(hr)) return hr;
        hr = State(device); if (FAILED(hr)) return hr;
        Restore restore{context};
        context->SwapDeviceContextState(captureState.Get(),restore.previous.GetAddressOf());
        context->ClearState();
        D3D11_TEXTURE2D_DESC desc = {}; depth->GetDesc(&desc);
        if (desc.SampleDesc.Count != 1 || !(desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)) return E_INVALIDARG;
        D3D11_SHADER_RESOURCE_VIEW_DESC view = {};
        view.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D; view.Texture2D.MipLevels = 1;
        view.Format = desc.Format == DXGI_FORMAT_R24G8_TYPELESS ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_R32_FLOAT;
        Ptr<ID3D11ShaderResourceView> depthSrv,motionSrv;
        hr=device->CreateShaderResourceView(depth,&view,depthSrv.GetAddressOf()); if(FAILED(hr))return hr;
        hr=device->CreateShaderResourceView(motion,nullptr,motionSrv.GetAddressOf()); if(FAILED(hr))return hr;
        ID3D11ShaderResourceView* srvs[]={depthSrv.Get(),motionSrv.Get()};
        ID3D11UnorderedAccessView* uavs[]={depthUav.Get(),motionUav.Get()};
        context->CSSetShader(conversion.Get(),nullptr,0); context->CSSetShaderResources(0,2,srvs);
        context->CSSetUnorderedAccessViews(0,2,uavs,nullptr);
        context->Dispatch((renderWidth+7)/8,(renderHeight+7)/8,1);
        scene=true;
        return S_OK;
    }
    HRESULT ProtectUi(ID3D11Device1* device, ID3D11DeviceContext1* context, ID3D11Texture2D* finalColor)
    {
        // Opaque protection of changed pixels, not recovered translucent UI alpha.
        // Dilation also protects antialiased text edges; underlying scene is retained
        // from HUD-less color. Embedded/world UI is excluded by the eligibility gate.
        static const char code[] = R"(
Texture2D<float4> scene:register(t0); Texture2D<float4> finalColor:register(t1);
RWTexture2D<float> mask:register(u0);
[numthreads(8,8,1)] void main(uint3 p:SV_DispatchThreadID) {
 uint w,h; mask.GetDimensions(w,h); if(p.x>=w||p.y>=h)return;
 float changed=0;
 for(int y=-1;y<=1;y++)for(int x=-1;x<=1;x++){
  int2 q=clamp(int2(p.xy)+int2(x,y),int2(0,0),int2(w-1,h-1));
  float3 d=abs(scene.Load(int3(q,0)).rgb-finalColor.Load(int3(q,0)).rgb);
  changed=max(changed,step(0.5/255.0,max(d.x,max(d.y,d.z)))); }
 mask[p.xy]=changed;
})";
        HRESULT hr=Shader(device,code,sizeof(code)-1,uiMask); if(FAILED(hr))return hr;
        hr=State(device); if(FAILED(hr))return hr;
        Restore restore{context}; context->SwapDeviceContextState(captureState.Get(),restore.previous.GetAddressOf());
        context->ClearState();
        Ptr<ID3D11ShaderResourceView> before,after;
        hr=device->CreateShaderResourceView(render[2].Get(),nullptr,before.GetAddressOf()); if(FAILED(hr))return hr;
        hr=device->CreateShaderResourceView(finalColor,nullptr,after.GetAddressOf()); if(FAILED(hr))return hr;
        ID3D11ShaderResourceView* srvs[]={before.Get(),after.Get()};
        ID3D11UnorderedAccessView* uav=uiUav.Get();
        context->CSSetShader(uiMask.Get(),nullptr,0); context->CSSetShaderResources(0,2,srvs);
        context->CSSetUnorderedAccessViews(0,1,&uav,nullptr);
        context->Dispatch((displayWidth+7)/8,(displayHeight+7)/8,1);
        return S_OK;
    }
};
