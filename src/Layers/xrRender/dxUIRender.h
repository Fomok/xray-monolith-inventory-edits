#ifndef	dxUIRender_included
#define	dxUIRender_included
#pragma once

#include "..\..\Include\xrRender\UIRender.h"

class dxUIRender : public IUIRender
{
public:
	dxUIRender() : PrimitiveType(ptNone), m_PointType(pttNone) { ; }

	virtual void CreateUIGeom();
	virtual void DestroyUIGeom();

	virtual void SetShader(IUIShader& shader);
	virtual void SetAlphaRef(int aref);
	//.	virtual void StartTriList(u32 iMaxVerts);
	//.	virtual void FlushTriList();
	//.	virtual void StartTriFan(u32 iMaxVerts);
	//.	virtual void FlushTriFan();
	//virtual void StartTriStrip(u32 iMaxVerts);
	//virtual void FlushTriStrip();
	//.	virtual void StartLineStrip(u32 iMaxVerts);
	//.	virtual void FlushLineStrip();
	//.	virtual void StartLineList(u32 iMaxVerts);
	//.	virtual void FlushLineList();
	virtual void SetScissor(Irect* rect = NULL);
	virtual void GetActiveTextureResolution(Fvector2& res);

	//.	virtual void PushPoint(float x, float y, u32 c, float u, float v);
	//	virtual void PushPoint(int x, int y, u32 c, float u, float v);
	virtual void PushPoint(float x, float y, float z, u32 C, float u, float v);

	virtual void StartPrimitive(u32 iMaxVerts, ePrimitiveType primType, ePointType pointType);
	virtual void FlushPrimitive();

	virtual LPCSTR UpdateShaderName(LPCSTR tex_name, LPCSTR sh_name);

	virtual void CacheSetXformWorld(const Fmatrix& M);
	virtual void CacheSetCullMode(CullMode);
	virtual bool SupportsFlatBackground() const;
	virtual void DrawFlatBackground(u32 color, float distance);


    virtual bool SupportsWorkbench() const;
    virtual void SetWorkbenchActive(bool active) { m_workbenchActive = active; }
    virtual bool WorkbenchActive() const { return m_workbenchActive && SupportsWorkbench(); }
    virtual bool BeginWorkbenchUI();
    virtual bool BeginWorkbenchModel(bool compose);
    virtual void EndWorkbenchPass();

private:
    bool m_workbenchActive = false;
#if defined(USE_DX11)
    ref_rt m_wbPosition, m_wbColor, m_wbDepth, m_wbModel, m_wbUI;
    ref_shader m_wbCompose;
    ID3DRenderTargetView* m_wbSavedRT[4] = {};
    ID3DDepthStencilView* m_wbSavedDepth = nullptr;
    bool m_wbPass = false;
    void EnsureWorkbenchTargets(bool model);
    void SaveWorkbenchTargets();
    void DrawWorkbenchQuad();
#endif

	ref_geom hGeom_TL;
	ref_geom hGeom_LIT;
	ref_shader m_flatBackgroundShader;

	ePrimitiveType PrimitiveType;
	ePointType m_PointType;

	//	Vertex buffer attributes
	u32 m_iMaxVerts;
	u32 vOffset;

	FVF::TL* TL_start_pv;
	FVF::TL* TL_pv;

	FVF::LIT* LIT_start_pv;
	FVF::LIT* LIT_pv;
};

extern dxUIRender UIRenderImpl;

#endif	//	dxUIRender_included
