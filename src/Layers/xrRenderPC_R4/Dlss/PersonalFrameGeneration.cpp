#include "stdafx.h"
#include "PersonalFrameGeneration.h"
#include "PersonalFgEligibility.h"
#include "../../xrRenderDX10/dx12PresentationBridge.h"
#include "../../../xrEngine/IGame_Persistent.h"
#include "../../../xrEngine/IGame_Level.h"
#include "../../../xrEngine/CustomHUD.h"

// Camera/Reflex sequencing adapted from Brodrigz 1.0.2 FrameGeneration.cpp.
// Credits: Brodrigz, OGSR team, xrSimpAI. Bodycam eligibility is deliberately stricter.
extern int ps_r_dlss_fg, ps_framelimiter;
extern float scope_radius;
extern bool IsMainMenuActive();
namespace personal_fg
{
namespace
{
bool started=false, submitted=false, hidden=false, previous=false;
Fmatrix lastTransform;
Fvector lastPosition, lastDirection;
u32 lastFrame=0, lastWidth=0, lastHeight=0;
float lastFov=0;
const char* lastStatus=nullptr;
void Status(const char* reason)
{
    if(reason!=lastStatus && ps_r_dlss_fg)Msg("[Personal FG] %s",reason);
    lastStatus=reason;
}
bool Eligible()
{
    if(!ps_r_dlss_fg){Status("Off");return false;}
    auto* bridge=HW.presentationBridge;
    if(!bridge || !bridge->GenerationReady()){Status("Unavailable: launch with SQA_DLSS_FG=1; check capability log; restart after a failure.");return false;}
    if(!Device.b_is_Active || Device.Paused() || !g_pGameLevel || Device.dwPrecacheFrame || IsMainMenuActive())
    {Status("Suspended for menu/loading/pause/inactive window.");return false;}
    const float aim = g_pGamePersistent && g_pGamePersistent->m_pGShaderConstants ?
        g_pGamePersistent->m_pGShaderConstants->hud_params.x : 0.f;
    if(ScopeActive(Device.m_SecondViewport.IsSVPActive(), scope_fake_enabled != 0,
        scope_3D_fake_enabled != 0, aim, scope_radius) || ps_r2_nightvision>0 || ps_r2_heatvision>0)
    {Status("Suspended for scopes/night vision/thermal; native rendered frames preserved.");return false;}
    if(RImplementation.GMBase.RGraph.hasNativeHudAttachments ||
        (g_hud && (g_hud->RenderActiveItemUIQuery() || g_hud->RenderCamAttachedUIQuery())))
    {Status("Suspended for embedded item/camera UI.");return false;}
    if(RImplementation.o.dx10_msaa || !RImplementation.o.ssfx_motionvectors || !dlss::Active())
    {Status("Requires active DLSS/FSR upscaling, SSS motion vectors and MSAA off.");return false;}
    Status("Inputs eligible; actual generation is reported by DLSS-G PostPresent diagnostics.");return true;
}
void Check(HRESULT result)
{
    CHECK_OR_EXIT(SUCCEEDED(result), "Personal frame generation synchronization failed; restart without SQA_DLSS_FG.");
}
}
bool ReflexLimitsFrames()
{
    return HW.presentationBridge && HW.presentationBridge->ReflexReady() &&
        Device.b_is_Active && !Device.Paused() && !IsMainMenuActive();
}
void BeginSimulation()
{
    started=true;submitted=false;
    auto& svp=Device.m_SecondViewport;
    hidden=!Device.true_pip_on && svp.IsSVPActive() && (Device.dwFrame+1)%svp.GetSVPFrameDelay()==0;
    auto* bridge=HW.presentationBridge;
    if(!bridge || hidden)return;
    // Existing r__framelimit remains a rendered-FPS cap. Reflex owns that cap
    // when active; do not also busy-wait in the engine.
    const UINT limit=ReflexLimitsFrames() && ps_framelimiter>0 ? UINT(1000000.f/ps_framelimiter) : 0;
    Check(bridge->BeginFrame(limit));
}
void BeginRender()
{
    if(!started)BeginSimulation();
    if(submitted || hidden)return;
    submitted=true;
    if(HW.presentationBridge)HW.presentationBridge->EndSimulation();
}
void EndPresent(){started=submitted=hidden=false;}
void Reset(){previous=started=submitted=hidden=false;lastStatus=nullptr;if(HW.presentationBridge)HW.presentationBridge->DiscardInputs();}
void CaptureScene(ID3D11Texture2D* depth, ID3D11Texture2D* motion)
{
    if(Device.true_pip_on && Device.m_SecondViewport.m_render_pass_is_svp)return;
    if(!Eligible()){previous=false;return;}
    const bool cut=!previous || Device.dwFrame!=lastFrame+1 || lastWidth!=dlss::RenderWidth() ||
        lastHeight!=dlss::RenderHeight() || lastPosition.distance_to_sqr(Device.vCameraPosition)>64.f ||
        lastDirection.dotproduct(Device.vCameraDirection)<.25f || _abs(lastFov-Device.fFOV)>10.f;
    Fmatrix invProjection,invCurrent,toPrevious,fromPrevious;
    D3DXMatrixInverse((D3DXMATRIX*)&invProjection,nullptr,(D3DXMATRIX*)&Device.mProject);
    D3DXMatrixInverse((D3DXMATRIX*)&invCurrent,nullptr,(D3DXMATRIX*)&Device.mFullTransform);
    toPrevious.mul(cut ? Device.mFullTransform : lastTransform,invCurrent);
    D3DXMatrixInverse((D3DXMATRIX*)&fromPrevious,nullptr,(D3DXMATRIX*)&toPrevious);
    sl::Constants c{};
    static_assert(sizeof(c.cameraViewToClip)==sizeof(Fmatrix),"matrix layout");
    memcpy(&c.cameraViewToClip,&Device.mProject,sizeof(Fmatrix));
    memcpy(&c.clipToCameraView,&invProjection,sizeof(Fmatrix));
    memcpy(&c.clipToPrevClip,&toPrevious,sizeof(Fmatrix));
    memcpy(&c.prevClipToClip,&fromPrevious,sizeof(Fmatrix));
    Fmatrix identity;identity.identity();memcpy(&c.clipToLensClip,&identity,sizeof(Fmatrix));
    const auto jitter=dlss::RasterJitter();c.jitterOffset={jitter.x,jitter.y};
    c.mvecScale={-1.f,-1.f};c.cameraPinholeOffset={0.f,0.f};
    auto vector=[](const Fvector& v){return sl::float3{v.x,v.y,v.z};};
    c.cameraPos=vector(Device.vCameraPosition);c.cameraUp=vector(Device.vCameraTop);
    c.cameraRight=vector(Device.vCameraRight);c.cameraFwd=vector(Device.vCameraDirection);
    c.cameraNear=-Device.mProject._43/Device.mProject._33;
    c.cameraFar=-Device.mProject._43/(Device.mProject._33-1.f);
    c.cameraFOV=2.f*atanf(1.f/Device.mProject._22);c.cameraAspectRatio=Device.mProject._22/Device.mProject._11;
    c.depthInverted=sl::eFalse;c.cameraMotionIncluded=sl::eTrue;
    c.motionVectors3D=c.motionVectorsJittered=sl::eFalse;c.reset=cut ? sl::eTrue : sl::eFalse;
    const HRESULT result=HW.presentationBridge->CaptureScene(depth,motion,c);
    if(FAILED(result)){Msg("! [Personal FG] input capture failed 0x%08x",result);Check(HW.presentationBridge->FailGeneration());previous=false;return;}
    previous=result==S_OK;lastFrame=Device.dwFrame;lastTransform=Device.mFullTransform;
    lastWidth=dlss::RenderWidth();lastHeight=dlss::RenderHeight();
    lastPosition=Device.vCameraPosition;lastDirection=Device.vCameraDirection;lastFov=Device.fFOV;
}
void CaptureHudless()
{
    if(HW.presentationBridge)HW.presentationBridge->CaptureHudless();
}
}
