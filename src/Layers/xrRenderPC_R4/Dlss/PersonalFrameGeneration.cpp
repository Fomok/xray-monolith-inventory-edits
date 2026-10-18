#include "stdafx.h"
#include "PersonalFrameGeneration.h"
#include "PersonalFgEligibility.h"
#include "../../xrRenderDX10/dx12PresentationBridge.h"
#include "../../../xrEngine/IGame_Persistent.h"
#include "../../../xrEngine/IGame_Level.h"
#include "../../../xrEngine/CustomHUD.h"

// Camera/Reflex sequencing adapted from Brodrigz 1.0.2 FrameGeneration.cpp.
// Credits: Brodrigz, OGSR team, xrSimpAI. Bodycam eligibility is deliberately stricter.
extern u32 ps_r_dlss_fg, ps_r_dlss_fg_pip;
extern int ps_framelimiter;
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
u32 lastScopeEpoch=~0u, lastScopeSession=~0u;
bool lastScoped=false;
dlss::Offset lastScopeMainJitter{};
float lastScopeFov=0;
u32 lastScopeWidth=0,lastScopeHeight=0;
float lastFov=0;
const char* lastStatus=nullptr;
u32 lastUiFlags=~u32(0);
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
    const auto* main=RImplementation.TargetMain;
    const auto* scope=RImplementation.TargetSVP;
    const bool lens=!RImplementation.GMBase.RGraph.mapScopeHUDSorted.empty();
    const bool scoped=ScopeActive(Device.m_SecondViewport.IsSVPActive(), scope_fake_enabled != 0,
        lens, aim, scope_radius);
    if(ps_r2_nightvision>0 || ps_r2_heatvision>0)
    {Status("Suspended for night vision/thermal.");return false;}
    if(scoped && !ps_r_dlss_fg_pip)
    {Status("Suspended in scopes: PiP frame generation is off in Video settings.");return false;}
    if(scoped && (!Device.true_pip_on || !main || !scope ||
        main->scope_reference_composited_frame!=Device.dwFrame ||
        scope->scope_reference_raw_frame!=Device.dwFrame || !main->rt_scope_reference_capture_tc || !scope->rt_baseZB))
    {Status("Suspended: scope has no current optical mapping/depth; legacy scopes are unsupported.");return false;}
    const u32 uiFlags = (g_hud ? g_hud->FrameGenerationUiFlags() : 0) |
        (RImplementation.GMBase.RGraph.hasNativeHudAttachments ? 16u : 0u);
    if (uiFlags != lastUiFlags)
    {
        Msg("[Personal FG] UI content flags=0x%X (1=item,2=item attachment,4=HUD attachment,8=camera UI,16=wearable surface)",uiFlags);
        lastUiFlags=uiFlags;
    }
    // Actor HUD widgets and wearable glass are composed after the clean-scene
    // snapshot. Item/camera UI still uses an earlier render path and must block.
    if(uiFlags & (1u | 2u | 8u))
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
bool NeedsHudless()
{
    return HW.presentationBridge && HW.presentationBridge->NeedsHudless();
}
bool ProtectEmbeddedUi()
{
    return ps_r_dlss_fg && HW.presentationBridge && HW.presentationBridge->GenerationReady() &&
        dlss::Active() && g_hud && (g_hud->FrameGenerationUiFlags() & 4u);
}
void BeginSimulation()
{
    started=true;submitted=false;
    auto& svp=Device.m_SecondViewport;
    hidden=!Device.true_pip_on && svp.IsSVPActive() && (Device.dwFrame+1)%svp.GetSVPFrameDelay()==0;
    auto* bridge=HW.presentationBridge;
    if(!bridge || hidden)return;
    // Reflex applies this target to PRESENTED FPS with FG, and rendered FPS
    // without FG. Preserve the user's target; do not also busy-wait in the engine.
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
    const bool scoped=Device.m_SecondViewport.IsSVPActive() || !RImplementation.GMBase.RGraph.mapScopeHUDSorted.empty();
    auto& svp=Device.m_SecondViewport;
    const bool scopeCut=scoped!=lastScoped || (scoped &&
        (lastScopeEpoch!=svp.svp_optic_epoch || lastScopeSession!=svp.GetSVPSession() ||
        _abs(lastScopeFov-svp.svp_fov)>.001f || lastScopeWidth!=RImplementation.TargetSVP->Width || lastScopeHeight!=RImplementation.TargetSVP->Height));
    const bool cut=scopeCut || !previous || Device.dwFrame!=lastFrame+1 || lastWidth!=dlss::RenderWidth() ||
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
    PersonalFgScope scopeInputs;
    if(scoped) {
        auto* target=RImplementation.TargetSVP;
        scopeInputs.mapping=RImplementation.TargetMain->rt_scope_reference_capture_tc->pSurface;
        scopeInputs.depth=target->rt_baseZB->pSurface;
        scopeInputs.motion=target->rt_ssfx_motion_vectors->pSurface;
        scopeInputs.mainMotion=motion;
        scopeInputs.requireGlass=RImplementation.TargetMain->scope_glass_deferred_current();
        Fmatrix current,old,inverse,reprojection;
        current.mul(Device.matrices[1].mProject,Device.matrices[1].mView);
        old.mul(Device.matrices_previous[1].mProject,Device.matrices_previous[1].mView);
        if(!D3DXMatrixInverse((D3DXMATRIX*)&inverse,nullptr,(D3DXMATRIX*)&current))
        {Status("Suspended: invalid scope camera projection.");previous=false;return;}
        reprojection.mul(old,inverse);
        memcpy(scopeInputs.reprojection,&reprojection,sizeof(Fmatrix));
        scopeInputs.planes[0]=svp.svp_near;scopeInputs.planes[1]=svp.svp_far;
        scopeInputs.planes[2]=c.cameraNear;scopeInputs.planes[3]=c.cameraFar;
        scopeInputs.mainJitterDelta[0]=(jitter.x-lastScopeMainJitter.x)/dlss::RenderWidth();
        scopeInputs.mainJitterDelta[1]=(jitter.y-lastScopeMainJitter.y)/dlss::RenderHeight();
        scopeInputs.reset=cut;
    }
    const HRESULT result=HW.presentationBridge->CaptureScene(depth,motion,c,scoped?&scopeInputs:nullptr);
    if(FAILED(result)){Msg("! [Personal FG] input capture failed 0x%08x",result);Check(HW.presentationBridge->FailGeneration());previous=false;return;}
    previous=result==S_OK;lastFrame=Device.dwFrame;lastTransform=Device.mFullTransform;
    lastWidth=dlss::RenderWidth();lastHeight=dlss::RenderHeight();
    lastPosition=Device.vCameraPosition;lastDirection=Device.vCameraDirection;lastFov=Device.fFOV;
    lastScoped=scoped;lastScopeEpoch=svp.svp_optic_epoch;lastScopeSession=svp.GetSVPSession();
    lastScopeMainJitter=jitter;lastScopeFov=svp.svp_fov;
    if(scoped){lastScopeWidth=RImplementation.TargetSVP->Width;lastScopeHeight=RImplementation.TargetSVP->Height;}
}
void CaptureHudless()
{
    if(HW.presentationBridge)HW.presentationBridge->CaptureHudless();
}
}
