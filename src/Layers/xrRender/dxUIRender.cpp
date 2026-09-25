#include "stdafx.h"
#include "dxUIRender.h"

#include "dxUIShader.h"

dxUIRender UIRenderImpl;

void dxUIRender::CreateUIGeom()
{
	hGeom_TL.create(FVF::F_TL, RCache.Vertex.Buffer(), 0);
	hGeom_LIT.create(FVF::F_LIT, RCache.Vertex.Buffer(), 0);
}

void dxUIRender::DestroyUIGeom()
{

    for (auto& it : g_UIShadersCache)
        it.second.destroy();
    g_UIShadersCache.clear();

	hGeom_TL = NULL;
	hGeom_LIT = NULL;
	m_flatBackgroundShader.destroy();
#if defined(USE_DX11)
    m_wbCompose.destroy();m_wbPosition.destroy();m_wbColor.destroy();
    m_wbDepth.destroy();m_wbModel.destroy();m_wbUI.destroy();
    m_wbPass=false;
#endif
}

void dxUIRender::SetShader(IUIShader& shader)
{
	dxUIShader* pShader = (dxUIShader*)&shader;
	VERIFY(&pShader);
	VERIFY(pShader->hShader);
	RCache.set_Shader(pShader->hShader);
}

void dxUIRender::SetAlphaRef(int aref)
{
	//CHK_DX(HW.pDevice->SetRenderState(D3DRS_ALPHAREF,aref));
	RCache.set_AlphaRef(aref);
}

/*
void dxUIRender::StartTriList(u32 iMaxVerts)
{
	VERIFY(PrimitiveType==ptNone);
	m_PointType = pttLIT;
	m_iMaxVerts = iMaxVerts;
	start_pv	= (FVF::LIT*)RCache.Vertex.Lock	(m_iMaxVerts,hGeom_fan.stride(),vOffset);
	pv			= start_pv;
	PrimitiveType = ptTriList;
}

void dxUIRender::FlushTriList()
{
	VERIFY(PrimitiveType==ptTriList);
	VERIFY(u32(pv-start_pv)<=m_iMaxVerts);

	std::ptrdiff_t p_cnt		= (pv-start_pv)/3;							
	RCache.Vertex.Unlock		(u32(pv-start_pv),hGeom_fan.stride());
	RCache.set_Geometry			(hGeom_fan);
	if (p_cnt!=0)RCache.Render	(D3DPT_TRIANGLELIST,vOffset,u32(p_cnt));

	PrimitiveType = ptNone;
}

void dxUIRender::StartTriFan(u32 iMaxVerts)
{
	VERIFY(PrimitiveType==ptNone);
	m_iMaxVerts = iMaxVerts;
	start_pv	= (FVF::LIT*)RCache.Vertex.Lock	(m_iMaxVerts,hGeom_fan.stride(),vOffset);
	pv			= start_pv;
	PrimitiveType = ptTriFan;
	m_PointType	= pttLIT;

}

void dxUIRender::FlushTriFan()
{
	VERIFY(PrimitiveType==ptTriFan);
	VERIFY(u32(pv-start_pv)<=m_iMaxVerts);

	std::ptrdiff_t p_cnt		= pv-start_pv;
	RCache.Vertex.Unlock		(u32(p_cnt),hGeom_fan.stride());
	RCache.set_Geometry	 		(hGeom_fan);
	if (p_cnt>2) RCache.Render	(D3DPT_TRIANGLEFAN,vOffset,u32(p_cnt-2));

	PrimitiveType = ptNone;
}

void dxUIRender::StartTriStrip(u32 iMaxVerts)
{
	VERIFY(PrimitiveType==ptNone);
	m_iMaxVerts = iMaxVerts;
	start_pv	= (FVF::TL*)RCache.Vertex.Lock	(m_iMaxVerts,hGeom_fan.stride(),vOffset);
	pv			= start_pv;
	PrimitiveType = ptTriStrip;
}

void dxUIRender::FlushTriStrip()
{
}


void dxUIRender::StartLineStrip(u32 iMaxVerts)
{
	VERIFY(PrimitiveType==ptNone);
	m_iMaxVerts = iMaxVerts;
	start_pv	= (FVF::LIT*)RCache.Vertex.Lock	(m_iMaxVerts,hGeom_fan.stride(),vOffset);
	pv			= start_pv;
	PrimitiveType = ptLineStrip;
	m_PointType = pttLIT;
}

void dxUIRender::FlushLineStrip()
{
	VERIFY(PrimitiveType==ptLineStrip);
	VERIFY(u32(pv-start_pv)<=m_iMaxVerts);

	std::ptrdiff_t p_cnt		= pv-start_pv;
	RCache.Vertex.Unlock		(u32(p_cnt),hGeom_fan.stride());
	RCache.set_Geometry	 		(hGeom_fan);
	if (p_cnt>1) RCache.Render	(D3DPT_LINESTRIP,vOffset,u32(p_cnt-1));

	PrimitiveType = ptNone;
}

void dxUIRender::StartLineList(u32 iMaxVerts)
{
	VERIFY(PrimitiveType==ptNone);
	m_iMaxVerts = iMaxVerts;
	start_pv	= (FVF::LIT*)RCache.Vertex.Lock	(m_iMaxVerts,hGeom_fan.stride(),vOffset);
	pv			= start_pv;
	PrimitiveType = ptLineList;
}

void dxUIRender::FlushLineList()
{
	VERIFY(PrimitiveType==ptLineList);
	VERIFY(u32(pv-start_pv)<=m_iMaxVerts);

	std::ptrdiff_t p_cnt		= pv-start_pv;
	RCache.Vertex.Unlock		(u32(p_cnt),hGeom_fan.stride());
	RCache.set_Geometry	 		(hGeom_fan);
	if (p_cnt>1) RCache.Render	(D3DPT_LINELIST,vOffset,u32(p_cnt)/2);

	PrimitiveType = ptNone;
}
*/
void dxUIRender::SetScissor(Irect* rect)
{
#if (RENDER == R_R3) || (RENDER == R_R4)
	RCache.set_Scissor(rect);
	StateManager.OverrideScissoring(rect ? true : false, TRUE);
#else	//	(RENDER == R_R3) || (RENDER == R_R4)
	RCache.set_Scissor(rect);
#endif	//	(RENDER == R_R3) || (RENDER == R_R4)
}

void dxUIRender::GetActiveTextureResolution(Fvector2& res)
{
	CTexture* T = RCache.get_ActiveTexture(0);
	res.set(float(T->get_Width()), float(T->get_Height()));
}

LPCSTR dxUIRender::UpdateShaderName(LPCSTR tex_name, LPCSTR sh_name)
{
	string_path buff;
	u32 v_dev = CAP_VERSION(HW.Caps.raster_major, HW.Caps.raster_minor);
	u32 v_need = CAP_VERSION(2, 0);
	if ((v_dev >= v_need) && FS.exist(buff, "$game_textures$", tex_name, ".ogm"))
		return "hud\\movie";
	else
		return sh_name;
}

/*
void dxUIRender::PushPoint(float x, float y, u32 c, float u, float v)
{
	VERIFY(m_PointType==pttNone);
	pv->set(x, y, 0.0f, c, u, v);
	++pv;
}
*/
/*
void dxUIRender::PushPoint(int x, int y, u32 c, float u, float v)
{
	VERIFY(m_PointType==pttNone);
	pv->set(x, y, 0, c, u, v);
	++pv;
}
*/

void dxUIRender::PushPoint(float x, float y, float z, u32 C, float u, float v)
{
	//.	VERIFY(m_PointType==pttLIT);
	switch (m_PointType)
	{
	case pttLIT:
		LIT_pv->set(x, y, z, C, u, v);
		++LIT_pv;
		break;
	case pttTL:
		TL_pv->set(x, y, C, u, v);
		++TL_pv;
		break;
	}
}

void dxUIRender::StartPrimitive(u32 iMaxVerts, ePrimitiveType primType, ePointType pointType)
{
	VERIFY(PrimitiveType==ptNone);
	VERIFY(m_PointType==pttNone);
	//.	R_ASSERT(pointType==pttLIT);

	m_iMaxVerts = iMaxVerts;
	PrimitiveType = primType;
	m_PointType = pointType;

	switch (m_PointType)
	{
	case pttLIT:
		LIT_start_pv = (FVF::LIT*)RCache.Vertex.Lock(m_iMaxVerts, hGeom_LIT.stride(), vOffset);
		LIT_pv = LIT_start_pv;
		break;
	case pttTL:
		TL_start_pv = (FVF::TL*)RCache.Vertex.Lock(m_iMaxVerts, hGeom_TL.stride(), vOffset);
		TL_pv = TL_start_pv;
		break;
	}
}

void dxUIRender::FlushPrimitive()
{
	u32 primCount = 0;
	_D3DPRIMITIVETYPE d3dPrimType = D3DPT_FORCE_DWORD;
	std::ptrdiff_t p_cnt = 0;

	switch (m_PointType)
	{
	case pttLIT:
		p_cnt = LIT_pv - LIT_start_pv;
		VERIFY(u32(p_cnt)<=m_iMaxVerts);

		RCache.Vertex.Unlock(u32(p_cnt), hGeom_LIT.stride());
		RCache.set_Geometry(hGeom_LIT);
		break;
	case pttTL:
		p_cnt = TL_pv - TL_start_pv;
		VERIFY(u32(p_cnt)<=m_iMaxVerts);

		RCache.Vertex.Unlock(u32(p_cnt), hGeom_TL.stride());
		RCache.set_Geometry(hGeom_TL);
		break;
	default:
		NODEFAULT;
	}

	//	Update data for primitive type
	switch (PrimitiveType)
	{
	case ptTriStrip:
		primCount = (u32)(p_cnt - 2);
		d3dPrimType = D3DPT_TRIANGLESTRIP;
		break;
	case ptTriList:
		primCount = (u32)(p_cnt / 3);
		d3dPrimType = D3DPT_TRIANGLELIST;
		break;
	case ptLineStrip:
		primCount = (u32)(p_cnt - 1);
		d3dPrimType = D3DPT_LINESTRIP;
		break;
	case ptLineList:
		primCount = (u32)(p_cnt / 2);
		d3dPrimType = D3DPT_LINELIST;
		break;
	default:
		NODEFAULT;
	}

	if (primCount > 0)
		RCache.Render(d3dPrimType, vOffset, primCount);

	PrimitiveType = ptNone;
	m_PointType = pttNone;
}

void dxUIRender::CacheSetXformWorld(const Fmatrix& M)
{
	RCache.set_xform_world(M);
}

void dxUIRender::CacheSetCullMode(CullMode m)
{
	RCache.set_CullMode(CULL_NONE + m);
}

// Keep this pass out of the scene's lighting, bloom, tone mapping and SSR.
// It runs in the camera-attachment UI pass, after phase_combine. The
// attachment geometry has already populated the near (0..0.02) depth range.
#if defined(USE_DX11)
class CBlender_FlatUIBackground : public IBlender
{
public:
	virtual LPCSTR getComment() { return "Flat inspection background"; }
	virtual BOOL canBeLMAPped() { return FALSE; }
	virtual void Compile(CBlender_Compile& C)
	{
		IBlender::Compile(C);
		if (C.iElement != 0) return;
		const bool msaa = RImplementation.o.dx10_msaa;
		// Use the three-argument overload, then set states explicitly. Passing
		// bool/BOOL values in the longer call is ambiguous with the GS overload
		// on MSVC (false can also match the geometry-shader name argument).
		C.r_Pass("fmk_ui_background", "fmk_ui_background", false);
		C.PassSET_ZB(!msaa, FALSE);
		C.PassSET_Blend(msaa, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, FALSE, 0);
		if (msaa) C.r_dx10Texture("s_inspection_depth", "$user$msaadepth");
		C.r_End();
	}
};
#endif

bool dxUIRender::SupportsFlatBackground() const
{
#if defined(USE_DX11)
	return HW.FeatureLevel >= D3D_FEATURE_LEVEL_11_0;
#else
	return false;
#endif
}

void dxUIRender::DrawFlatBackground(u32 color, float distance)
{
#if defined(USE_DX11)
	if (!SupportsFlatBackground() || !_valid(distance) || distance <= 0.f) return;
	VERIFY(PrimitiveType == ptNone);
	if (!m_flatBackgroundShader)
	{
		CBlender_FlatUIBackground blender;
		m_flatBackgroundShader.create(&blender, "fmk_ui_background");
	}
	// Use the same camera projection and viewport depth range as the gun.
	// Full-screen clip coordinates avoid the ordinary 2D UI transform path.
	const Fmatrix& P = Device.mProject;
	const float w = P._34 * distance + P._44;
	if (w <= EPS) return;
	const float z = (P._33 * distance + P._43) / w;
	if (z < 0.f || z > 1.f) return;
	u32 offset;
	FVF::LIT* v = (FVF::LIT*)RCache.Vertex.Lock(4, hGeom_LIT.stride(), offset);
	color |= 0xff000000;
	v[0].set(-1.f, -1.f, z, color, 0.f, 1.f);
	v[1].set(-1.f,  1.f, z, color, 0.f, 0.f);
	v[2].set( 1.f, -1.f, z, color, 1.f, 1.f);
	v[3].set( 1.f,  1.f, z, color, 1.f, 0.f);
	RCache.Vertex.Unlock(4, hGeom_LIT.stride());
	RCache.set_Element(m_flatBackgroundShader->E[0]);
	RCache.set_Geometry(hGeom_LIT);
	RCache.set_CullMode(CULL_NONE);
	RCache.set_Stencil(FALSE);
	RCache.Render(D3DPT_TRIANGLESTRIP, offset, 2);
	RCache.set_CullMode(CULL_CCW);
#endif
}

// Experimental PDA workbench: separate single-sample model G-buffer and UI
// texture. The world/PDA render targets are restored after every pass.
bool dxUIRender::SupportsWorkbench() const
{
#if defined(USE_DX11)
    return SupportsFlatBackground() && !RImplementation.o.dx10_msaa;
#else
    return false;
#endif
}
#if defined(USE_DX11)
class CBlender_Workbench : public IBlender
{
public:
    virtual LPCSTR getComment() { return "PDA workbench studio"; }
    virtual BOOL canBeLMAPped() { return FALSE; }
    virtual void Compile(CBlender_Compile& C)
    {
        IBlender::Compile(C);
        C.r_Pass("fmk_ui_background", "fmk_pda_workbench", false);
        C.PassSET_ZB(FALSE,FALSE);
        C.PassSET_Blend(FALSE,D3DBLEND_ONE,D3DBLEND_ZERO,FALSE,0);
        C.r_dx10Texture("s_wb_position","$user$fmk_wb_position");
        C.r_dx10Texture("s_wb_color","$user$fmk_wb_color");
        C.r_End();
    }
};
void dxUIRender::EnsureWorkbenchTargets(bool model)
{
    const u32 w=Device.dwWidth,h=Device.dwHeight;
    if (!m_wbUI) m_wbUI.create("$user$fmk_workbench",w,h,D3DFMT_A8R8G8B8);
    // Status, crafting and showcase need only the UI target. Defer the model
    // buffers until the first actual preview, then reuse them across tabs.
    if (!model || m_wbModel) return;
    m_wbPosition.create("$user$fmk_wb_position",w,h,D3DFMT_A16B16G16R16F);
    m_wbColor.create("$user$fmk_wb_color",w,h,D3DFMT_A16B16G16R16F);
    m_wbDepth.create("$user$fmk_wb_depth",w,h,D3DFMT_D24S8);
    m_wbModel.create("$user$fmk_wb_model",w,h,D3DFMT_A8R8G8B8);
    const FLOAT clear[4]={0.025f,0.026f,0.024f,1.f};
    HW.pContext->ClearRenderTargetView(m_wbModel->pRT,clear);
    CBlender_Workbench blender;m_wbCompose.create(&blender,"fmk_pda_workbench");
}
void dxUIRender::SaveWorkbenchTargets()
{
    VERIFY(!m_wbPass);m_wbPass=true;
    for(u32 i=0;i<4;++i) m_wbSavedRT[i]=RCache.get_RT(i);
    m_wbSavedDepth=RCache.get_ZB();
}
void dxUIRender::DrawWorkbenchQuad()
{
    u32 offset;FVF::LIT* v=(FVF::LIT*)RCache.Vertex.Lock(4,hGeom_LIT.stride(),offset);
    v[0].set(-1,-1,0,0xffffffff,0,1);v[1].set(-1,1,0,0xffffffff,0,0);
    v[2].set(1,-1,0,0xffffffff,1,1);v[3].set(1,1,0,0xffffffff,1,0);
    RCache.Vertex.Unlock(4,hGeom_LIT.stride());
    RCache.set_Element(m_wbCompose->E[0]);RCache.set_Geometry(hGeom_LIT);
    RCache.set_CullMode(CULL_NONE);RCache.set_Stencil(FALSE);
    RCache.Render(D3DPT_TRIANGLESTRIP,offset,2);
}
#endif
bool dxUIRender::BeginWorkbenchModel(bool compose)
{
#if defined(USE_DX11)
    if(!WorkbenchActive()) return false;
    PROF_EVENT("PDA workbench model pass");
    EnsureWorkbenchTargets(true);SaveWorkbenchTargets();
    const FLOAT clear[4]={0,0,0,0};
    if(!compose)
    {
        HW.pContext->ClearRenderTargetView(m_wbPosition->pRT,clear);
        HW.pContext->ClearRenderTargetView(m_wbColor->pRT,clear);
        HW.pContext->ClearDepthStencilView(m_wbDepth->pZRT,D3D_CLEAR_DEPTH|D3D_CLEAR_STENCIL,1.f,0);
        RCache.set_RT(m_wbPosition->pRT,0);RCache.set_RT(m_wbColor->pRT,1);
        // The isolated studio shader reads only position and color. Discard
        // heat/motion outputs instead of writing two unused FP16 surfaces.
        RCache.set_RT(nullptr,2);RCache.set_RT(nullptr,3);
        RCache.set_ZB(m_wbDepth->pZRT);
    }
    else
    {
        RCache.set_RT(m_wbModel->pRT,0);
        for(u32 i=1;i<4;++i) RCache.set_RT(nullptr,i);
        RCache.set_ZB(m_wbDepth->pZRT);
        DrawWorkbenchQuad();
    }
    return true;
#else
    return false;
#endif
}
bool dxUIRender::BeginWorkbenchUI()
{
#if defined(USE_DX11)
    if(!SupportsWorkbench()) return false;
    EnsureWorkbenchTargets(false);SaveWorkbenchTargets();
    RCache.set_RT(m_wbUI->pRT,0);
    for(u32 i=1;i<4;++i) RCache.set_RT(nullptr,i);
    RCache.set_ZB(nullptr);RCache.set_Stencil(FALSE);
    if (m_wbModel)
        HW.pContext->CopyResource(m_wbUI->pSurface,m_wbModel->pSurface);
    else
    {
        const FLOAT clear[4]={0.025f,0.026f,0.024f,1.f};
        HW.pContext->ClearRenderTargetView(m_wbUI->pRT,clear);
    }
    return true;
#else
    return false;
#endif
}
void dxUIRender::EndWorkbenchPass()
{
#if defined(USE_DX11)
    if(!m_wbPass) return;
    for(u32 i=0;i<4;++i) RCache.set_RT(m_wbSavedRT[i],i);
    RCache.set_ZB(m_wbSavedDepth);RCache.set_CullMode(CULL_CCW);m_wbPass=false;
#endif
}
