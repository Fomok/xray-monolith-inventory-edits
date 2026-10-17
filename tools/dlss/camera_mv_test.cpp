// Execute the shipped fallback shader on WARP, with mixed world/HUD/lens data.
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <array>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdexcept>
using Microsoft::WRL::ComPtr;
static void Check(HRESULT hr) { if (FAILED(hr)) throw std::runtime_error("D3D11 call failed"); }
static void Require(bool ok) { if (!ok) throw std::runtime_error("Motion fallback mismatch"); }
int main() try
{
    ComPtr<ID3D11Device> d; ComPtr<ID3D11DeviceContext> c;
    Check(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,D3D11_CREATE_DEVICE_DEBUG,
        nullptr,0,D3D11_SDK_VERSION,&d,nullptr,&c));
    ComPtr<ID3D11InfoQueue> debug; Check(d.As(&debug));
    const char* vsSource = "struct O{float4 p:SV_Position;float2 uv:TEXCOORD0;}; O main(uint i:SV_VertexID){O o; o.uv=float2((i<<1)&2,i&2);o.p=float4(o.uv*float2(2,-2)+float2(-1,1),0,1);return o;}";
    ComPtr<ID3DBlob> vc, pc, err;
    Check(D3DCompile(vsSource,strlen(vsSource),nullptr,nullptr,nullptr,"main","vs_5_0",0,0,&vc,&err));
    HRESULT hr=D3DCompileFromFile(L"gamedata/shaders/r3/dlss_camera_mv.ps",nullptr,D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main","ps_5_0",D3DCOMPILE_ENABLE_STRICTNESS,0,&pc,&err);
    if(err) std::printf("%s",(char*)err->GetBufferPointer()); Check(hr);
    ComPtr<ID3D11VertexShader> vs; ComPtr<ID3D11PixelShader> ps;
    Check(d->CreateVertexShader(vc->GetBufferPointer(),vc->GetBufferSize(),nullptr,&vs));
    Check(d->CreatePixelShader(pc->GetBufferPointer(),pc->GetBufferSize(),nullptr,&ps));
    constexpr unsigned W=8,H=4;
    using Pixel=std::array<float,4>;
    std::array<float,W*H> depths; depths.fill(.8f);
    std::array<Pixel,W*H> mv{}, mask{};
    for(unsigned y=0;y<H;++y) {
        mv[y*W+0]={.125f,-.25f,0,0}; mv[y*W+1]={0,0,1,0};
        depths[y*W+2]=.01f; mv[y*W+3]={0,0,0,3};
        mask[y*W+4]={0,0,0,1}; depths[y*W+6]=1.f;
    }
    D3D11_TEXTURE2D_DESC td{}; td.Width=W;td.Height=H;td.MipLevels=td.ArraySize=td.SampleDesc.Count=1;
    td.BindFlags=D3D11_BIND_SHADER_RESOURCE;
    ComPtr<ID3D11Texture2D> depth, motion, lens, output, staging;
    auto texture=[&](auto& tex,DXGI_FORMAT format,const void* data,unsigned stride) {
        td.Format=format; D3D11_SUBRESOURCE_DATA init{data,stride,0};
        Check(d->CreateTexture2D(&td,data?&init:nullptr,&tex));
    };
    texture(depth,DXGI_FORMAT_R32_FLOAT,depths.data(),W*4);
    texture(motion,DXGI_FORMAT_R32G32B32A32_FLOAT,mv.data(),W*16);
    texture(lens,DXGI_FORMAT_R32G32B32A32_FLOAT,mask.data(),W*16);
    ComPtr<ID3D11ShaderResourceView> ds,ms,ls;
    Check(d->CreateShaderResourceView(depth.Get(),nullptr,&ds));
    Check(d->CreateShaderResourceView(motion.Get(),nullptr,&ms));
    Check(d->CreateShaderResourceView(lens.Get(),nullptr,&ls));
    td.BindFlags=D3D11_BIND_RENDER_TARGET; texture(output,DXGI_FORMAT_R32G32B32A32_FLOAT,nullptr,0);
    ComPtr<ID3D11RenderTargetView> rt;Check(d->CreateRenderTargetView(output.Get(),nullptr,&rt));
    td.BindFlags=0;td.Usage=D3D11_USAGE_STAGING;td.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    texture(staging,DXGI_FORMAT_R32G32B32A32_FLOAT,nullptr,0);
    D3D11_BUFFER_DESC bd{};bd.ByteWidth=80;bd.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
    ComPtr<ID3D11Buffer> cb;Check(d->CreateBuffer(&bd,nullptr,&cb));
    c->VSSetShader(vs.Get(),nullptr,0);c->PSSetShader(ps.Get(),nullptr,0);
    c->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    c->PSSetConstantBuffers(0,1,cb.GetAddressOf());
    ID3D11ShaderResourceView* srvs[]={ds.Get(),ms.Get(),ls.Get()};c->PSSetShaderResources(0,3,srvs);
    c->OMSetRenderTargets(1,rt.GetAddressOf(),nullptr);
    D3D11_VIEWPORT vp{0,0,float(W),float(H),0,1};c->RSSetViewports(1,&vp);
    unsigned cases=0;
    for(float jitter:{-.49f,0.f,.49f}) for(int mode=0;mode<6;++mode) {
        // Column-major matrix: affine reprojection, near-zero W, behind camera,
        // extreme motion, no history, and disabled scope coverage.
        float params[20]={1.2f,0,0,0, 0,.8f,0,0, 0,0,1,0, .2f,.1f,0,1,
            jitter/W,-jitter/H,1,1};
        if(mode==1)params[15]=0;
        if(mode==2)params[15]=-1;
        if(mode==3)params[12]=100;
        if(mode==4)params[18]=0;
        if(mode==5)params[19]=0;
        c->UpdateSubresource(cb.Get(),0,nullptr,params,0,0);c->Draw(3,0);
        c->CopyResource(staging.Get(),output.Get());D3D11_MAPPED_SUBRESOURCE read{};
        Check(c->Map(staging.Get(),0,D3D11_MAP_READ,0,&read));
        for(unsigned y=0;y<H;++y)for(unsigned x=0;x<W;++x) {
            Pixel expected=mv[y*W+x];
            if((mode==0||mode==5)&&(x==5||x==7||(mode==5&&x==4))) {
                const float u=(x+.5f)/W-params[16],v=(y+.5f)/H-params[17];
                expected[0]=-.2f*u;expected[1]=.2f*v-.05f;
            }
            const float* actual=(const float*)((const char*)read.pData+y*read.RowPitch)+4*x;
            for(unsigned i=0;i<4;++i)Require(std::isfinite(actual[i])&&fabs(actual[i]-expected[i])<1e-5f);
        }
        c->Unmap(staging.Get(),0);++cases;
    }
    for(UINT64 i=0;i<debug->GetNumStoredMessages();++i) {
        SIZE_T size=0;debug->GetMessage(i,nullptr,&size);std::vector<char> bytes(size);
        auto* m=(D3D11_MESSAGE*)bytes.data();Check(debug->GetMessage(i,m,&size));
        if(m->Severity<=D3D11_MESSAGE_SEVERITY_WARNING){std::puts(m->pDescription);Require(false);}
    }
    c->ClearState();std::printf("Camera MV shader passed %u WARP cases: motion, jitter, HUD, lens, sky, tags and invalid projection.\n",cases);
} catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}
