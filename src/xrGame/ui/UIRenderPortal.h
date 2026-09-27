#pragma once
#include "UIPreviewTexture.h"

// A single owner is admitted by the renderer. Renewal expires after one frame
// without updates. Destruction/release cannot cancel another owner's request.
class CUIPreviewContext
{
    u32 m_color=0xff060706;
    shared_str m_texture;
    bool m_dry=false;
    float m_gain=1.f;
public:
    ~CUIPreviewContext() { Release(); }
    bool Activate(bool embedded)
    {
        if (!UIRender->AcquirePreview(this,embedded)) return false;
        UIRender->ConfigurePreview(this,m_color,m_texture.c_str(),m_dry,m_gain);
        return true;
    }
    bool RequestSceneSuppression() { return Activate(false); }
    void Release() { UIRender->ReleasePreview(this); }
    bool Active() const { return UIRender->IsPreviewOwner(this); }
    bool SceneSuppressed() const { return Active() && UIRender->SceneSuppressed(); }
    void SetBackgroundColor(u32 color) { m_color=color; }
    bool SetBackgroundTexture(LPCSTR texture)
    {
        m_texture=nullptr;
        if (!ValidatePreviewTexture(texture)) return false;
        if (texture && *texture) m_texture=texture;
        return true;
    }
    void SetDry(bool dry) { m_dry=dry; }
    bool SetLightingGain(float gain)
    {
        if (!_valid(gain) || gain<0.f || gain>8.f) return false;
        m_gain=gain;return true;
    }
};

class CUIRenderPortal : public CUIStatic
{
    // Strong Lua reference. Source must be Lua-owned (SetAutoDelete(false));
    // native-parent-owned windows must be detached before native destruction.
    luabind::object m_sourceRef;
    CUIDialogWndEx* m_source=nullptr;
    CUIPreviewContext m_context;
    bool m_texture=false,m_active=false;
    float m_width=1024.f,m_height=768.f;
    u32 m_updateFrame=u32(-1);
    struct CursorScope
    {
        Fvector2 saved;
        CursorScope(CUIRenderPortal* p)
        {
            CUICursor& c=UI().GetUICursor();saved=c.GetCursorPosition();
            Frect r;p->GetAbsoluteRect(r);Fvector2 mapped;
            mapped.set((saved.x-r.left)*p->m_width/_max(1.f,r.width()),
                (saved.y-r.top)*p->m_height/_max(1.f,r.height()));
            c.SetLogicalPosition(mapped);
        }
        ~CursorScope() { UI().GetUICursor().SetLogicalPosition(saved); }
    };
public:
    void SetSource(const luabind::object& source)
    {
        m_context.Release();m_source=nullptr;m_sourceRef=source;m_updateFrame=u32(-1);
        if (luabind::type(source)!=LUA_TNIL)
            m_source=luabind::object_cast<CUIDialogWndEx*>(source);
    }
    bool SetActive(bool active)
    {
        m_active=active && m_source && IsShown() && m_context.Activate(true);
        if (!m_active) m_context.Release();
        return m_active;
    }
    bool SetBackgroundTexture(LPCSTR name) { return m_context.SetBackgroundTexture(name); }
    void SetBackgroundColor(u32 color) { m_context.SetBackgroundColor(color); }
    void SetDry(bool dry) { m_context.SetDry(dry); }
    bool SetLightingGain(float gain) { return m_context.SetLightingGain(gain); }
    virtual void Update()
    {
        if(m_updateFrame==Device.dwFrame) return;
        m_updateFrame=Device.dwFrame;
        CUIStatic::Update();
        if(!m_active || !m_source || !IsShown() || !m_source->IsShown()) { m_context.Release();return; }
        if(!m_context.Activate(true)) { m_active=false;return; }
        CursorScope cursor(this);m_source->Update();
    }
    virtual bool OnMouseAction(float x,float y,EUIMessages action)
    {
        if(!m_source || !m_context.Active()) return false;
        CursorScope cursor(this);
        return m_source->OnMouseAction(x*m_width/_max(1.f,GetWidth()),y*m_height/_max(1.f,GetHeight()),action);
    }
    virtual bool OnKeyboardAction(int dik,EUIMessages action)
    {
        return m_source && m_context.Active() ? m_source->OnKeyboardAction(dik,action) : false;
    }
    virtual void Draw()
    {
        if(!m_source || !m_context.Active() || !IsShown()) return;
        UI().RenderFont();
        if(UIRender->BeginPreviewUI())
        {
            Frect full;full.set(0,0,m_width,m_height);UI().PushScissor(full,true);
            m_source->Draw();UI().RenderFont();UI().PopScissor();UIRender->EndPreviewPass();
        }
        if(!m_texture) { InitTexture("$user$ui_preview_surface");SetStretchTexture(true);m_texture=true; }
        CUIStatic::Draw();
    }
};

inline bool UIPreviewSupports(LPCSTR feature)
{
    if (!feature) return false;
    if (!xr_strcmp(feature,"background")) return UIRender->SupportsFlatBackground();
    if (!xr_strcmp(feature,"isolated_model") || !xr_strcmp(feature,"ui_portal") ||
        !xr_strcmp(feature,"scene_suppression")) return UIRender->SupportsModelPreview();
    return false;
}
