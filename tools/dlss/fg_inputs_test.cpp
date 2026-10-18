#define NOMINMAX
#include <windows.h>
#include <d3d11_4.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <vector>
#include <cstdio>
#include <cmath>
#include "../../src/Layers/xrRenderDX10/PersonalFgInputs.h"
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
using Microsoft::WRL::ComPtr;
static void Check(HRESULT hr,const char* step){if(FAILED(hr)){printf("FAIL %s 0x%08lx\n",step,hr);ExitProcess(1);}}
int main()
{
 ComPtr<ID3D11Device> raw;ComPtr<ID3D11DeviceContext> rawContext;D3D_FEATURE_LEVEL level;
 Check(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&raw,&level,&rawContext),"D3D11");
 ComPtr<ID3D11Device1> device;ComPtr<ID3D11DeviceContext1> context;
 Check(raw.As(&device),"device1");Check(rawContext.As(&context),"context1");
 ComPtr<IDXGIDevice> dxgi;ComPtr<IDXGIAdapter> adapter;Check(raw.As(&dxgi),"DXGI");Check(dxgi->GetAdapter(&adapter),"adapter");
 DXGI_ADAPTER_DESC adapterDesc{};Check(adapter->GetDesc(&adapterDesc),"description");
 wprintf(L"Adapter: %s\n",adapterDesc.Description);
 ComPtr<ID3D12Device> d12;Check(D3D12CreateDevice(adapter.Get(),D3D_FEATURE_LEVEL_11_0,IID_PPV_ARGS(&d12)),"D3D12");
 PersonalFgInputs inputs;
 Check(inputs.Resize(d12.Get(),device.Get(),64,64,80,64),"shared inputs");
 auto source=[&](DXGI_FORMAT format,const void* data,UINT pitch){
   D3D11_TEXTURE2D_DESC d{};d.Width=d.Height=64;d.MipLevels=d.ArraySize=1;d.Format=format;d.SampleDesc.Count=1;d.BindFlags=D3D11_BIND_SHADER_RESOURCE;
   D3D11_SUBRESOURCE_DATA initial{};initial.pSysMem=data;initial.SysMemPitch=pitch;
   ComPtr<ID3D11Texture2D> texture;Check(device->CreateTexture2D(&d,&initial,&texture),"source");return texture;
 };
 std::vector<float> depths(64*64,.5f),motions(64*64*4);
 for(UINT i=0;i<64*64;i++){motions[i*4]=.25f;motions[i*4+1]=-.125f;}
 auto depth=source(DXGI_FORMAT_R32_FLOAT,depths.data(),64*sizeof(float));
 auto motion=source(DXGI_FORMAT_R32G32B32A32_FLOAT,motions.data(),64*4*sizeof(float));
 ComPtr<ID3D11RenderTargetView> sceneRtv;
 Check(device->CreateRenderTargetView(inputs.render[2].Get(),nullptr,&sceneRtv),"scene RTV");
 ID3D11RenderTargetView* prior=sceneRtv.Get();context->OMSetRenderTargets(1,&prior,nullptr);
 Check(inputs.Capture(device.Get(),context.Get(),depth.Get(),motion.Get()),"convert shader");
 ComPtr<ID3D11RenderTargetView> restored;context->OMGetRenderTargets(1,&restored,nullptr);
 if(restored.Get()!=prior){puts("FAIL context-state restore");return 1;}
 D3D11_TEXTURE2D_DESC colorDesc{};inputs.render[2]->GetDesc(&colorDesc);
 colorDesc.MiscFlags=0;colorDesc.BindFlags=D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
 ComPtr<ID3D11Texture2D> finalColor;Check(device->CreateTexture2D(&colorDesc,nullptr,&finalColor),"final color");
 ComPtr<ID3D11RenderTargetView> finalRtv;Check(device->CreateRenderTargetView(finalColor.Get(),nullptr,&finalRtv),"final RTV");
 const float black[]={0,0,0,1},white[]={1,1,1,1};
 context->ClearRenderTargetView(sceneRtv.Get(),black);context->ClearRenderTargetView(finalRtv.Get(),black);
 D3D11_RECT rect{20,20,30,30};context->ClearView(finalRtv.Get(),white,&rect,1);
 Check(inputs.ProtectUi(device.Get(),context.Get(),finalColor.Get()),"UI mask shader");
 auto read=[&](UINT index,auto validate){
  D3D11_TEXTURE2D_DESC d{};inputs.render[index]->GetDesc(&d);d.BindFlags=d.MiscFlags=0;d.Usage=D3D11_USAGE_STAGING;d.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
  ComPtr<ID3D11Texture2D> staging;Check(device->CreateTexture2D(&d,nullptr,&staging),"staging");
  context->CopyResource(staging.Get(),inputs.render[index].Get());
  D3D11_MAPPED_SUBRESOURCE mapped{};Check(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped),"readback");
  bool ok=validate(mapped);context->Unmap(staging.Get(),0);if(!ok){printf("FAIL input %u readback\n",index);ExitProcess(1);}
 };
 read(0,[](const D3D11_MAPPED_SUBRESOURCE& m){return fabsf(*static_cast<const float*>(m.pData)-.5f)<1e-6f;});
 read(1,[](const D3D11_MAPPED_SUBRESOURCE& m){const auto* h=static_cast<const unsigned short*>(m.pData);return h[0]==0x3400 && h[1]==0xb000;});
 read(3,[](const D3D11_MAPPED_SUBRESOURCE& m){const auto* p=static_cast<const unsigned char*>(m.pData);return p[0]==0 && p[25*m.RowPitch+25]==255 && p[19*m.RowPitch+19]==255 && p[18*m.RowPitch+18]==0;});
 context->ClearState();context->Flush();
 puts("PASS depth=.5, RG16 motion=(.25,-.125), UI interior/edge/dilation/background and context-state restore.");
 return 0;
}
