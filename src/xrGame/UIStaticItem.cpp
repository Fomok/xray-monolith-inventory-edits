#include "stdafx.h"
#include "UIStaticItem.h"
#include "ui_base.h"


void CreateUIGeom()
{
	UIRender->CreateUIGeom();
}

void DestroyUIGeom()
{
	UIRender->DestroyUIGeom();
}

CUIStaticItem::CUIStaticItem()
{
	uFlags.zero();
	vSize.set(0, 0);
	TextureRect.set(0, 0, 0, 0);
	vHeadingPivot.set(0, 0);
	vHeadingOffset.set(0, 0);
	dwColor = 0xffffffff;
	m_fit = tfFill;
}

void CUIStaticItem::ComputeRenderUV(Frect& uv) const
{
	uv = TextureRect;
	if (m_fit != tfCover)
		return;

	float rw = TextureRect.width();
	float rh = TextureRect.height();
	if (rw <= 0.0f || rh <= 0.0f || vSize.y <= 0.0f)
		return;

	float aspect = vSize.x / vSize.y;
	float vw = rw;
	float vh = rh;
	if (rw / rh > aspect)
		vw = rh * aspect;
	else
		vh = rw / aspect;

	uv.x1 = TextureRect.x1 + (rw - vw) * 0.5f;
	uv.y1 = TextureRect.y1 + (rh - vh) * 0.5f;
	uv.x2 = uv.x1 + vw;
	uv.y2 = uv.y1 + vh;
}

void CUIStaticItem::ResetHeadingPivot()
{
	uFlags.set(flValidHeadingPivot, FALSE);
	uFlags.set(flFixedLTWhileHeading,FALSE);
}

void CUIStaticItem::SetHeadingPivot(const Fvector2& p, const Fvector2& offset, bool fixedLT)
{
	vHeadingPivot = p;
	vHeadingOffset = offset;
	uFlags.set(flValidHeadingPivot, TRUE);
	if (fixedLT)
		uFlags.set(flFixedLTWhileHeading,TRUE);
	else
		uFlags.set(flFixedLTWhileHeading,FALSE);
}

void CUIStaticItem::RenderInternal(const Fvector2& in_pos)
{
	Fvector2 pos;
	UI().ClientToScreenScaled(pos, in_pos.x, in_pos.y);
	UI().AlignPixel(pos.x);
	UI().AlignPixel(pos.y);

	Fvector2 ts;
	UIRender->GetActiveTextureResolution(ts);

	if (!uFlags.test(flValidSize))
		SetSize(ts);

	if (!uFlags.test(flValidTextureRect))
		SetTextureRect(Frect().set(0, 0, ts.x, ts.y));

	Fvector2 LTp, RBp;
	Fvector2 LTt, RBt;
	//координаты на экране в пикселях
	LTp.set(pos);

	UI().ClientToScreenScaled(RBp, vSize.x, vSize.y);
	RBp.add(pos);

	//текстурные координаты
	Frect uv;
	ComputeRenderUV(uv);
	LTt.set(uv.x1 / ts.x, uv.y1 / ts.y);
	RBt.set(uv.x2 / ts.x, uv.y2 / ts.y);

	float offset = -0.5f;
	if (UI().m_currentPointType == IUIRender::pttLIT)
		offset = 0.0f;

	// clip poly
	sPoly2D S;
	S.resize(4);

	LTp.x += offset;
	LTp.y += offset;
	RBp.x += offset;
	RBp.y += offset;

	S[0].set(LTp.x, LTp.y, LTt.x, LTt.y); // LT
	S[1].set(RBp.x, LTp.y, RBt.x, LTt.y); // RT
	S[2].set(RBp.x, RBp.y, RBt.x, RBt.y); // RB
	S[3].set(LTp.x, RBp.y, LTt.x, RBt.y); // LB

	sPoly2D D;
	sPoly2D* R = NULL;

	if (UI().m_currentPointType != IUIRender::pttLIT)
		R = UI().ActiveClipFrustum().ClipPoly(S, D);
	else
	{
		R = UI().ScreenFrustumLIT().ClipPoly(S, D);
	}

	RenderPolygon(R);
}

void CUIStaticItem::RenderInternal(float angle)
{
	Fvector2 ts;
	Fvector2 hp;

	UIRender->GetActiveTextureResolution(ts);
	hp.set(0.5f / ts.x, 0.5f / ts.y);

	if (!uFlags.test(flValidSize))
		SetSize(ts);

	if (!uFlags.test(flValidTextureRect))
		SetTextureRect(Frect().set(0, 0, ts.x, ts.y));

	Fvector2 pivot, offset, SZ;
	SZ.set(vSize);


	float cosA = _cos(angle);
	float sinA = _sin(angle);

	// Rotation
	if (!uFlags.test(flValidHeadingPivot))
		pivot.set(vSize.x / 2.f, vSize.y / 2.f);
	else
		pivot.set(vHeadingPivot.x, vHeadingPivot.y);

	offset.set(vPos);
	offset.add(vHeadingOffset);

	Fvector2 LTt, RBt;
	Frect uv;
	ComputeRenderUV(uv);
	LTt.set(uv.x1 / ts.x + hp.x, uv.y1 / ts.y + hp.y);
	RBt.set(uv.x2 / ts.x + hp.x, uv.y2 / ts.y + hp.y);

	float kx = UI().get_current_kx();

	// clip poly
	sPoly2D S;
	S.resize(4);
	// LT
	S[0].set(0.f, 0.f, LTt.x, LTt.y);
	S[0].rotate_pt(pivot, cosA, sinA, kx);
	S[0].pt.add(offset);

	// RT
	S[1].set(SZ.x, 0.f, RBt.x, LTt.y);
	S[1].rotate_pt(pivot, cosA, sinA, kx);
	S[1].pt.add(offset);
	// RB
	S[2].set(SZ.x, SZ.y, RBt.x, RBt.y);
	S[2].rotate_pt(pivot, cosA, sinA, kx);
	S[2].pt.add(offset);
	// LB
	S[3].set(0.f, SZ.y, LTt.x, RBt.y);
	S[3].rotate_pt(pivot, cosA, sinA, kx);
	S[3].pt.add(offset);

	for (int i = 0; i < 4; ++i)
		UI().ClientToScreenScaled(S[i].pt);

	sPoly2D D;
	sPoly2D* R = UI().ActiveClipFrustum().ClipPoly(S, D);
	RenderPolygon(R);
}

// Allocate GPU vertices only after clipping. Hidden icons used to lock and
// unlock a vertex buffer even when clipping produced no triangles.
void CUIStaticItem::RenderPolygon(const sPoly2D* polygon)
{
    if (!polygon || polygon->size() < 3)
        return;

    const u32 vertexCount = 3 * (polygon->size() - 2);
    UIRender->StartPrimitive(vertexCount, IUIRender::ptTriList, UI().m_currentPointType);
    for (u32 k = 0; k < polygon->size() - 2; ++k)
    {
        const S2DVert& a = (*polygon)[0];
        const S2DVert& b = (*polygon)[k + 1];
        const S2DVert& c = (*polygon)[k + 2];
        UIRender->PushPoint(a.pt.x, a.pt.y, 0, dwColor, a.uv.x, a.uv.y);
        UIRender->PushPoint(b.pt.x, b.pt.y, 0, dwColor, b.uv.x, b.uv.y);
        UIRender->PushPoint(c.pt.x, c.pt.y, 0, dwColor, c.uv.x, c.uv.y);
    }
    UIRender->FlushPrimitive();
}

//---from static-item

void CUIStaticItem::Render()
{
	VERIFY(g_bRendering);
	UIRender->SetShader(*hShader);
	RenderInternal(vPos);
}

void CUIStaticItem::Render(float angle)
{
	VERIFY(g_bRendering);

	UIRender->SetShader(*hShader);
	RenderInternal(angle);
}


void CUIStaticItem::CreateShader(LPCSTR tex, LPCSTR sh)
{
    hShader->create(sh, tex, !!uFlags.test(flNoShaderCache));

#ifdef DEBUG
	dbg_tex_name = tex;
#endif
	uFlags.set(flValidSize, FALSE);
	uFlags.set(flValidTextureRect, FALSE);
}


void CUIStaticItem::Init(LPCSTR tex, LPCSTR sh, float left, float top)
{
	uFlags.set(flValidSize, FALSE);
	CreateShader(tex, sh);
	SetPos(left, top);
}
