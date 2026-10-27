#define SQA3_EXCEPTION_BOUNDARY
#include "stdafx.h"
#include "InventoryNextView.h"
#include "UIScrollView.h"
#include "UIStatic.h"
#include "UIButton.h"
#include "../ui_base.h"
#include "../UICursor.h"
#include <algorithm>

namespace
{
// Child mouse routing and mouse capture can bypass a parent's hit test. Check
// every ancestor rectangle, including the outer scroll viewport, at the leaf.
bool InsideInputBounds(CUIWindow* window, const Fvector2& absolute)
{
    for (auto current = window; current; current = current->GetParent())
    {
        if (!current->IsShown() || !current->IsEnabled()) return false;
        Frect rect;
        current->GetAbsoluteRect(rect);
        // Half-open bounds: right/bottom edges belong to the next cell/control.
        if (!(absolute.x >= rect.left && absolute.x < rect.right &&
              absolute.y >= rect.top && absolute.y < rect.bottom)) return false;
    }
    return true;
}

void CancelInput(CUIWindow* window)
{
    window->ReleaseMouseCapture(); // existing engine API erases only this subtree
    if (window->CursorOverWindow()) window->OnFocusLost();
    if (auto button = smart_cast<CUIButton*>(window))
        button->SetButtonState(CUIButton::BUTTON_NORMAL);
    for (auto child : window->GetChildWndList()) CancelInput(child);
}

class InventoryNextButton final : public CUIButton
{
public:
    bool OnMouseAction(float x, float y, EUIMessages action) override
    {
        Frect rect;
        GetAbsoluteRect(rect);
        if (!InsideInputBounds(this, Fvector2().set(rect.left+x, rect.top+y)))
        {
            CancelInput(this);
            return false;
        }
        return CUIButton::OnMouseAction(x, y, action);
    }
    void Update() override
    {
        CUIButton::Update();
        // Native Update computes hover from the leaf rectangle alone. Clear it
        // when a scroll/row boundary clips the button, even without mouse motion.
        if (!InsideInputBounds(this, GetUICursor().GetCursorPosition())) CancelInput(this);
    }
};

// Native scissor, using the same absolute-coordinate contract as CUIScrollView.
class InventoryNextClip final : public CUIWindow
{
public:
    bool OnMouseAction(float x, float y, EUIMessages action) override
    {
        Frect rect;
        GetAbsoluteRect(rect);
        if (!InsideInputBounds(this, Fvector2().set(rect.left+x, rect.top+y)))
        {
            CancelInput(this);
            return false;
        }
        return CUIWindow::OnMouseAction(x, y, action);
    }
    void Draw() override
    {
        Frect rect;
        GetAbsoluteRect(rect);
        UI().PushScissor(rect);
        CUIWindow::Draw();
        UI().PopScissor();
    }
};

void Place(CUIWindow* window, float x, float y, float width, float height)
{
    window->SetWndPos(Fvector2().set(x, y));
    window->SetWndSize(Fvector2().set(width, height));
    window->Show(true);
}

constexpr u32 textColor = 0xffe0e0d8;
constexpr u32 controlColor = 0xffa8d7ef;
}

CInventoryNextView::CInventoryNextView() = default;
CInventoryNextView::~CInventoryNextView()
{
    // Base destructor owns/deletes controls. No live model/entity was borrowed.
    m_actions.clear();
}

void CInventoryNextView::Initialize(const Fvector2& position, const Fvector2& size) noexcept try {
    SetWndPos(position);
    SetWndSize(Fvector2().set((std::max)(size.x, 300.0f), (std::max)(size.y, 160.0f)));
    if (!m_scroll)
    {
        m_scroll = xr_new<CUIScrollView>();
        m_scroll->SetAutoDelete(true);
        AttachChild(m_scroll);
        Place(m_scroll, 0, 0, GetWidth(), GetHeight());
        m_scroll->InitScrollView();
    }
    else
    {
        Place(m_scroll, 0, 0, GetWidth(), GetHeight());
        m_scroll->InitScrollView();
    }
    Show(true);
    Invalidate();
} catch (...) { FailClosed(); }

bool CInventoryNextView::Bind(const inventory_next::Model& model,
    const inventory_next::Presentation& state, inventory_next::Id actor, const Visuals& visuals) noexcept try {
    // Copy first: source may be destroyed immediately after this call returns.
    auto snapshot = std::unique_ptr<const inventory_next::Model>(new inventory_next::Model(model));
    auto presentation = state;
    const auto found = snapshot->nodes().find(actor);
    if (found == snapshot->nodes().end() || found->second.kind != inventory_next::Kind::ActorRoot ||
        !presentation.open(*snapshot, actor))
    {
        Unbind(); // mismatched session/revision/actor fails closed
        return false;
    }
    m_snapshot = std::move(snapshot);
    m_state = std::move(presentation);
    m_actor = actor;
    m_visuals = visuals;
    Invalidate();
    return true;
} catch (...) { FailClosed(); return false; }

bool CInventoryNextView::Refresh(const inventory_next::Model& model,
    const inventory_next::Presentation& state, inventory_next::Id actor, const Visuals& visuals) noexcept try {
    return Bind(model, state, actor, visuals);
} catch (...) { FailClosed(); return false; }

void CInventoryNextView::Unbind() noexcept try {
    m_snapshot.reset();
    m_state = inventory_next::Presentation{};
    m_visuals.clear();
    m_actor = 0;
    Invalidate();
} catch (...) { FailClosed(); }

void CInventoryNextView::Invalidate() noexcept try {
    m_dirty = true;
    m_actions.clear();
    // Cancel gestures owned by this subtree, without disturbing another dialog.
    for (unsigned attempts = 0; attempts < 128; ++attempts)
    {
        auto captured = CUIWindow::MouseCapturer();
        if (!captured) break;
        auto ancestor = captured;
        while (ancestor && ancestor != this) ancestor = ancestor->GetParent();
        if (ancestor != this) break;
        captured->ReleaseMouseCapture();
    }
    if (m_scroll) { m_scroll->Enable(false); m_scroll->Show(false); }
} catch (...) { FailClosed(); }

std::uint64_t CInventoryNextView::BoundSession() const noexcept try { return m_snapshot ? m_snapshot->session() : 0; } catch (...) { const_cast<CInventoryNextView*>(this)->FailClosed(); return 0; }
std::uint64_t CInventoryNextView::BoundRevision() const noexcept try { return m_snapshot ? m_snapshot->revision() : 0; } catch (...) { const_cast<CInventoryNextView*>(this)->FailClosed(); return 0; }

CUIStatic* CInventoryNextView::Text(CUIWindow* parent, const std::string& value,
    float x, float y, float width, float height, u32 color)
{
    auto text = xr_new<CUIStatic>();
    text->SetAutoDelete(true);
    parent->AttachChild(text);
    Place(text, x, y, width, height);
    text->TextureOff();
    text->TextItemControl()->SetFont(UI().Font().pFontLetterica16Russian);
    text->TextItemControl()->SetTextComplexMode(false);
    text->TextItemControl()->SetEllipsis(true);
    text->TextItemControl()->SetTextColor(color);
    text->TextItemControl()->SetText(value.c_str());
    text->Enable(false);
    return text;
}

void CInventoryNextView::Button(CUIWindow* parent, const std::string& value,
    float x, float y, float width, const Action& action)
{
    auto button = xr_new<InventoryNextButton>();
    button->SetAutoDelete(true);
    parent->AttachChild(button);
    Place(button, x, y, width, 24.0f);
    button->TextureOff();
    button->TextItemControl()->SetFont(UI().Font().pFontLetterica16Russian);
    button->TextItemControl()->SetTextColor(controlColor);
    button->TextItemControl()->SetText(value.c_str());
    button->SetMessageTarget(this);
    m_actions.emplace(button, action);
}

std::string CInventoryNextView::Label(inventory_next::Id id) const
{
    const auto visual = m_visuals.find(id);
    if (visual != m_visuals.end())
    {
        if (!visual->second.label.empty()) return visual->second.label;
        if (!visual->second.section.empty()) return visual->second.section;
    }
    if (m_snapshot)
    {
        const auto node = m_snapshot->nodes().find(id);
        if (node != m_snapshot->nodes().end())
        {
            const auto nativeSection = node->second.attributes.find("native_section");
            if (nativeSection != node->second.attributes.end() && !nativeSection->second.empty())
                return nativeSection->second;
            const auto section = node->second.attributes.find("section");
            if (section != node->second.attributes.end() && !section->second.empty()) return section->second;
        }
    }
    return "#" + std::to_string(id); // actual model ID, never a generated item
}

void CInventoryNextView::Rebuild()
{
    m_dirty = false;
    if (!m_scroll) return;
    const int previousScroll = m_scroll->GetCurrentScrollPos();
    m_actions.clear();
    m_scroll->Clear();
    const float width = (std::max)(260.0f, m_scroll->GetDesiredChildWidth() - 8.0f);
    auto header = xr_new<CUIWindow>();
    Place(header, 0, 0, width, 28.0f);
    m_scroll->AddWindow(header, true);
    Text(header, m_snapshot ? "Inventory - read only" : "Inventory unavailable", 4, 2, width-8, 24, textColor);
    if (!m_snapshot) { m_scroll->Show(true); m_scroll->Enable(true); return; }
    const auto& model = *m_snapshot;
    const auto snapshot = model.snapshot();
    std::set<unsigned> groups;
    for (const auto& item : snapshot)
        if (item.parent == m_actor && item.location == inventory_next::Location::Equipment)
            groups.insert(item.presentation_group);
    for (unsigned group : groups)
    {
        const auto selected = m_state.cycle(model, group, 0);
        auto row = xr_new<CUIWindow>();
        Place(row, 0, 0, width, 28);
        m_scroll->AddWindow(row, true);
        Text(row, "Equipment " + std::to_string(group) + ": " + Label(selected), 4, 2, width-120, 24, textColor);
        Button(row, "[<]", width-110, 0, 32, {ActionKind::Cycle, 0, -1, group});
        Button(row, "[>]", width-74, 0, 32, {ActionKind::Cycle, 0, 1, group});
        const auto node = model.nodes().find(selected);
        if (node != model.nodes().end() && node->second.container)
            Button(row, "[+]", width-38, 0, 32, {ActionKind::Open, selected, 0, 0});
    }
    for (const auto& panel : m_state.panels(model))
    {
        const auto found = model.nodes().find(panel.container);
        if (found == model.nodes().end()) continue;
        const auto grid = found->second.grid;
        // Scale presentation only. Model positions, masks and footprints unchanged.
        const float cell = (std::min)(m_cell, (width-12.0f) / (std::max)(1, grid.w));
        const unsigned rows = (std::min)(panel.visible_rows, static_cast<unsigned>(grid.h));
        auto block = xr_new<CUIWindow>();
        Place(block, 0, 0, width, panel.collapsed ? 30.0f : 34.0f + rows*cell);
        m_scroll->AddWindow(block, true);
        Text(block, Label(panel.container) + "  row " + std::to_string(panel.first_row+1), 4, 2, width-120, 24, textColor);
        Button(block, panel.collapsed ? "[+]" : "[-]", width-110, 0, 32, {ActionKind::Collapse, panel.container, 0, 0});
        Button(block, "[up]", width-74, 0, 32, {ActionKind::Scroll, panel.container, -1, 0});
        Button(block, "[dn]", width-38, 0, 32, {ActionKind::Scroll, panel.container, 1, 0});
        if (panel.collapsed) continue;
        auto viewport = xr_new<InventoryNextClip>();
        viewport->SetAutoDelete(true);
        block->AttachChild(viewport);
        Place(viewport, 4, 30, grid.w*cell, rows*cell);
        for (unsigned y=0; y<rows; ++y) for (int x=0; x<grid.w; ++x)
        {
            const auto region = model.cell_region(panel.container, x, static_cast<int>(panel.first_row+y));
            Text(viewport, region ? "." : "x", x*cell, y*cell, cell, cell, region ? 0xff707070 : 0xff805050);
        }
        for (const auto& item : m_state.visible_items(model, panel.container))
        {
            auto tile = xr_new<InventoryNextClip>();
            tile->SetAutoDelete(true);
            viewport->AttachChild(tile);
            Place(tile, item.position.x*cell, (item.position.y-static_cast<int>(panel.first_row))*cell,
                  item.footprint.w*cell, item.footprint.h*cell);
            const auto visual = m_visuals.find(item.id);
            if (visual != m_visuals.end() && !visual->second.texture_id.empty() && !item.position.rotated)
            {
                auto icon = xr_new<CUIStatic>();
                icon->SetAutoDelete(true);
                tile->AttachChild(icon);
                Place(icon, 0, 0, tile->GetWidth(), tile->GetHeight());
                icon->InitTexture(visual->second.texture_id.c_str());
                icon->SetStretchTexture(true);
                icon->Enable(false);
            }
            Text(tile, Label(item.id) + " x" + std::to_string(item.quantity), 1, 1, tile->GetWidth()-2, 20, textColor);
            const auto node = model.nodes().find(item.id);
            if (node != model.nodes().end() && node->second.container)
                Button(tile, "[+]", 0, (std::max)(0.0f, tile->GetHeight()-24), (std::min)(32.0f, tile->GetWidth()),
                       {ActionKind::Open, item.id, 0, 0});
        }
    }
    m_scroll->ForceUpdate();
    m_scroll->SetScrollPos(previousScroll);
    m_scroll->Enable(true);
    m_scroll->Show(true);
}

void CInventoryNextView::Update() noexcept try {
    if (m_dirty) Rebuild(); // never delete a button inside its own SendMessage stack
    CUIWindow::Update();
} catch (...) { FailClosed(); }

void CInventoryNextView::SendMessage(CUIWindow* sender, s16 message, void* data) noexcept try {
    const auto found = m_actions.find(sender);
    if (message != BUTTON_CLICKED || found == m_actions.end() || !m_snapshot)
    {
        CUIWindow::SendMessage(sender, message, data);
        return;
    }
    const auto action = found->second;
    switch (action.kind)
    {
    case ActionKind::Collapse:
        for (const auto& panel : m_state.panels(*m_snapshot))
            if (panel.container == action.id) { m_state.collapse(action.id, !panel.collapsed); break; }
        break;
    case ActionKind::Scroll: m_state.scroll(*m_snapshot, action.id, action.direction); break;
    case ActionKind::Cycle: m_state.cycle(*m_snapshot, action.group, action.direction); break;
    case ActionKind::Open: m_state.open(*m_snapshot, action.id); break;
    }
    Invalidate();
} catch (...) { FailClosed(); }

void CInventoryNextView::FailClosed() noexcept {
    // No allocation or authoritative model mutation while handling a failure.
    CancelInput(this);m_actions.clear();m_snapshot.reset();m_dirty=false;m_actor=0;
    if(m_scroll){m_scroll->Enable(false);m_scroll->Show(false);}
    Msg("! SQA3 native view invalidated after a presentation failure");
}
inventory_next::Presentation CInventoryNextView::PresentationState() const noexcept try {
    return m_state;
} catch (...) { const_cast<CInventoryNextView*>(this)->FailClosed();return {}; }
