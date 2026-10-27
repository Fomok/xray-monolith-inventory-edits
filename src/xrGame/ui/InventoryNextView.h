#pragma once

#include "UIWindow.h"
#include "../../inventory_next/presentation.h"
#include <memory>

class CUIScrollView;
class CUIStatic;

// Explicit opt-in renderer. Owns a value snapshot, never engine entity pointers.
// All entry points, including destruction, belong to the native UI thread.
class CInventoryNextView final : public CUIWindow
{
public:
    struct ItemVisual
    {
        std::string section;
        std::string label;
        // Optional, already validated registered CUI texture descriptor ID.
        // Empty means text fallback. This is not a guessed texture filename.
        std::string texture_id;
    };
    using Visuals = std::map<inventory_next::Id, ItemVisual>;

    CInventoryNextView();
    ~CInventoryNextView() override;
    void Initialize(const Fvector2& position, const Fvector2& size);
    bool Bind(const inventory_next::Model& model, const inventory_next::Presentation& state,
              inventory_next::Id actor, const Visuals& visuals);
    // Same binding contract as Bind. Caller supplies synchronized permissions.
    bool Refresh(const inventory_next::Model& model, const inventory_next::Presentation& state,
                 inventory_next::Id actor, const Visuals& visuals);
    void Unbind();
    // Hide stale data immediately; rebuild on next UI Update, not in a callback.
    void Invalidate();
    inventory_next::Presentation PresentationState() const { return m_state; }
    std::uint64_t BoundSession() const;
    std::uint64_t BoundRevision() const;
    void Update() override;
    void SendMessage(CUIWindow* sender, s16 message, void* data = nullptr) override;

private:
    enum class ActionKind { Collapse, Scroll, Cycle, Open };
    struct Action
    {
        ActionKind kind;
        inventory_next::Id id;
        int direction;
        unsigned group;
    };
    void Rebuild();
    CUIStatic* Text(CUIWindow* parent, const std::string& value,
                   float x, float y, float width, float height, u32 color);
    void Button(CUIWindow* parent, const std::string& value, float x, float y,
                float width, const Action& action);
    std::string Label(inventory_next::Id id) const;

    std::unique_ptr<const inventory_next::Model> m_snapshot;
    inventory_next::Presentation m_state;
    inventory_next::Id m_actor = 0;
    Visuals m_visuals;
    CUIScrollView* m_scroll = nullptr; // auto-deleted by this CUIWindow
    std::map<CUIWindow*, Action> m_actions; // child identities only, cleared before deletion
    bool m_dirty = false;
    float m_cell = 32.0f;
};
