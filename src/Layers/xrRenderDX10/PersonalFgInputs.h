#pragma once
// Shared-input transport for the personal DLSS-G bridge. Depth/motion conversion
// follows Brodrigz 1.0.2 / OGSR (xrSimpAI); UI protection is specific to this test.
#include <d3d11_4.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include "PersonalFgScope.h"
#pragma comment(lib, "d3dcompiler.lib")

class PersonalFgInputs
{
    template<class T> using Ptr = Microsoft::WRL::ComPtr<T>;
    Ptr<ID3DDeviceContextState> captureState;
    Ptr<ID3D11ComputeShader> conversion, uiMask;
    Ptr<ID3D11ComputeShader> scopeConversion;
    Ptr<ID3D11Texture2D> previousScopeMap;
    Ptr<ID3D11Buffer> scopeConstants;
    Ptr<ID3D11SamplerState> scopeSampler;
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
        Invalidate(); previousScopeMap.Reset(); depthUav.Reset(); motionUav.Reset(); uiUav.Reset();
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
    HRESULT Capture(ID3D11Device1* device, ID3D11DeviceContext1* context, ID3D11Texture2D* depth, ID3D11Texture2D* motion,
        const PersonalFgScope* scope=nullptr)
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
        if(scope) { hr=ComposeScope(device,context,*scope); if(FAILED(hr))return hr; }
        else previousScopeMap.Reset();
        scene=true;
        return S_OK;
    }
    HRESULT ComposeScope(ID3D11Device1* device, ID3D11DeviceContext1* context, const PersonalFgScope& scope)
    {
        static const char code[]=R"(
Texture2D<float4> mapping:register(t0), previousMapping:register(t1);
Texture2D<float> scopeDepth:register(t2);
Texture2D<float4> scopeMotion:register(t3);
SamplerState linearClamp:register(s0);
RWTexture2D<float> depthOut:register(u0);
RWTexture2D<float2> motionOut:register(u1);
cbuffer Params:register(b0) { row_major float4x4 reproject; float4 planes; float4 control; };
[numthreads(8,8,1)] void main(uint3 p:SV_DispatchThreadID) {
 uint w,h; depthOut.GetDimensions(w,h); if(p.x>=w||p.y>=h)return;
 float4 map=mapping.Load(int3(p.xy,0)); if(map.z<0.5)return;
 float2 uv=(p.xy+0.5)/float2(w,h), tc=map.xy;
 float d=scopeDepth.SampleLevel(linearClamp,tc,0);
 float z=planes.x*planes.y/max(planes.y-d*(planes.y-planes.x),1e-6);
 depthOut[p.xy]=saturate(planes.w/(planes.w-planes.z)-planes.z*planes.w/((planes.w-planes.z)*max(z,planes.z)));
 if(control.x>0.5) { motionOut[p.xy]=0; return; }
 float2 mv=scopeMotion.SampleLevel(linearClamp,tc,0).xy;
 if(dot(abs(mv),float2(1,1))<1e-6) {
   float4 old=mul(float4(tc.x*2-1,1-tc.y*2,d,1),reproject);
   if(old.w<=0) {motionOut[p.xy]=0;return;}
   mv=tc-float2(old.x/old.w*.5+.5,.5-old.y/old.w*.5);
 }
 float2 target=tc-mv;
 // Invert the PREVIOUS optical mapping, not the current lens scale. This
 // includes lens translation, rotation, parallax and optical distortion.
 float2 oldUV=uv-motionOut[p.xy]-control.yz;
 float2 texel=1.0/float2(w,h);
 bool valid=true;
 [unroll] for(int i=0;i<6;i++) {
   float4 c=previousMapping.SampleLevel(linearClamp,oldUV,0);
   float4 x=previousMapping.SampleLevel(linearClamp,oldUV+float2(texel.x,0),0);
   float4 y=previousMapping.SampleLevel(linearClamp,oldUV+float2(0,texel.y),0);
   float2 a=(x.xy-c.xy)/texel.x,b=(y.xy-c.xy)/texel.y,e=c.xy-target;
   float det=a.x*b.y-a.y*b.x;
   if(min(c.z,min(x.z,y.z))<.99 || abs(det)<1e-6){valid=false;break;}
   float2 step=float2(b.y*e.x-b.x*e.y,-a.y*e.x+a.x*e.y)/det;
   oldUV-=clamp(step,-.05,.05);
 }
 float4 solved=previousMapping.SampleLevel(linearClamp,oldUV,0);
 valid=valid && solved.z>.99 && all(oldUV>=0) && all(oldUV<=1) && length(solved.xy-target)<.002;
 // Newly exposed lens pixels have no predecessor; zero vectors leave their
 // correspondence to optical flow instead of leaking main-camera motion.
 motionOut[p.xy]=valid ? uv-oldUV-control.yz : float2(0,0);
})";
        if(!scope.mapping || !scope.depth || !scope.motion)return E_INVALIDARG;
        D3D11_TEXTURE2D_DESC desc{}; scope.mapping->GetDesc(&desc);
        if(desc.Width!=renderWidth || desc.Height!=renderHeight)return E_INVALIDARG;
        HRESULT hr=Shader(device,code,sizeof(code)-1,scopeConversion);if(FAILED(hr))return hr;
        bool reset=scope.reset || !previousScopeMap;
        if(previousScopeMap) { D3D11_TEXTURE2D_DESC old{};previousScopeMap->GetDesc(&old);
            if(old.Width!=desc.Width || old.Height!=desc.Height || old.Format!=desc.Format){previousScopeMap.Reset();reset=true;} }
        if(!previousScopeMap) {
            desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;desc.MiscFlags=0;
            hr=device->CreateTexture2D(&desc,nullptr,previousScopeMap.GetAddressOf());if(FAILED(hr))return hr;
        }
        if(reset)context->CopyResource(previousScopeMap.Get(),scope.mapping);
        struct Params {float matrix[16],planes[4],control[4];} params{};
        memcpy(params.matrix,scope.reprojection,sizeof(params.matrix));memcpy(params.planes,scope.planes,sizeof(params.planes));params.control[0]=reset?1.f:0.f;
        params.control[1]=scope.mainJitterDelta[0];params.control[2]=scope.mainJitterDelta[1];
        if(!scopeConstants) {D3D11_BUFFER_DESC b{};b.ByteWidth=sizeof(params);b.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
            hr=device->CreateBuffer(&b,nullptr,scopeConstants.GetAddressOf());if(FAILED(hr))return hr;}
        context->UpdateSubresource(scopeConstants.Get(),0,nullptr,&params,0,0);
        if(!scopeSampler) {D3D11_SAMPLER_DESC s{};s.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;s.AddressU=s.AddressV=s.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;s.MaxLOD=D3D11_FLOAT32_MAX;
            hr=device->CreateSamplerState(&s,scopeSampler.GetAddressOf());if(FAILED(hr))return hr;}
        Ptr<ID3D11ShaderResourceView> map,oldMap,depth,motion;
        hr=device->CreateShaderResourceView(scope.mapping,nullptr,&map);if(FAILED(hr))return hr;
        hr=device->CreateShaderResourceView(previousScopeMap.Get(),nullptr,&oldMap);if(FAILED(hr))return hr;
        D3D11_TEXTURE2D_DESC dd{};scope.depth->GetDesc(&dd);
        D3D11_SHADER_RESOURCE_VIEW_DESC dv{};dv.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;dv.Texture2D.MipLevels=1;
        dv.Format=dd.Format==DXGI_FORMAT_R24G8_TYPELESS?DXGI_FORMAT_R24_UNORM_X8_TYPELESS:DXGI_FORMAT_R32_FLOAT;
        hr=device->CreateShaderResourceView(scope.depth,&dv,&depth);if(FAILED(hr))return hr;
        hr=device->CreateShaderResourceView(scope.motion,nullptr,&motion);if(FAILED(hr))return hr;
        ID3D11ShaderResourceView* views[]={map.Get(),oldMap.Get(),depth.Get(),motion.Get()};
        ID3D11Buffer* cb=scopeConstants.Get();ID3D11SamplerState* sampler=scopeSampler.Get();
        context->CSSetShader(scopeConversion.Get(),nullptr,0);context->CSSetShaderResources(0,4,views);
        context->CSSetConstantBuffers(0,1,&cb);context->CSSetSamplers(0,1,&sampler);
        context->Dispatch((renderWidth+7)/8,(renderHeight+7)/8,1);
        ID3D11ShaderResourceView* empty[4]{};context->CSSetShaderResources(0,4,empty);
        context->CopyResource(previousScopeMap.Get(),scope.mapping);
        return S_OK;
    }
    HRESULT ProtectUi(ID3D11Device1* device, ID3D11DeviceContext1* context, ID3D11Texture2D* finalColor)
    {
        // Opaque protection of changed pixels, not recovered translucent UI alpha.
        // Dilation also protects antialiased text edges; underlying scene is retained
        // from HUD-less color. Native actor HUD widgets/glass are composed after
        // that capture too; item/camera UI remains excluded by eligibility.
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
