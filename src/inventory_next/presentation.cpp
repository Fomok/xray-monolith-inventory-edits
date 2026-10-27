#include "presentation.h"
#include <algorithm>
#include <limits>

namespace inventory_next {
bool Presentation::current(const Model& m) const {
    return session_ != 0 && session_ == m.session() && revision_ == m.revision();
}
bool Presentation::accessible(const Model& m, Id id) const {
    const auto found = m.nodes().find(id);
    if (found == m.nodes().end() || !found->second.container) return false;
    if (carried(m, id)) return true;
    const Node* node = &found->second;
    // Model already validates the tree; bound traversal defensively anyway.
    for (unsigned depth = 0; depth <= 64; ++depth) {
        if (!node->parent) return node->kind == Kind::WorldRoot && world_.count(id) != 0;
        const auto parent = m.nodes().find(node->parent);
        if (parent == m.nodes().end()) return false;
        node = &parent->second;
    }
    return false;
}
bool Presentation::carried(const Model& m, Id id) const {
    for (unsigned depth = 0; depth <= 64; ++depth) {
        if (actor_ != 0 && id == actor_) return true;
        const auto it = m.nodes().find(id);
        if (it == m.nodes().end() || !it->second.parent) return false;
        id = it->second.parent;
    }
    return false;
}
std::vector<Id> Presentation::group(const Model& m, unsigned value) const {
    std::vector<Id> ids;
    for (const auto& pair : m.nodes()) {
        const auto& n = pair.second;
        if (n.parent == actor_ && n.location == Location::Equipment && n.presentation_group == value)
            ids.push_back(n.id);
    }
    return ids;
}
void Presentation::clamp(const Model& m, PanelState& p) const {
    const auto it = m.nodes().find(p.container);
    if (it == m.nodes().end()) { p.first_row = 0; return; }
    const auto height = static_cast<unsigned>(std::max(0, it->second.grid.h));
    const auto last = height > p.visible_rows ? height - p.visible_rows : 0;
    p.first_row = std::min(p.first_row, last);
}
void Presentation::synchronize(const Model& m, Id actor, const std::set<Id>& world) {
    if (session_ != m.session() || actor_ != actor) { panels_.clear(); selected_.clear(); }
    actor_ = actor;
    const auto found = m.nodes().find(actor);
    if (found == m.nodes().end() || found->second.kind != Kind::ActorRoot) actor_ = 0;
    session_ = m.session(); revision_ = m.revision(); world_ = world;
    panels_.erase(std::remove_if(panels_.begin(), panels_.end(), [&](const PanelState& p) {
        return !accessible(m, p.container);
    }), panels_.end());
    for (auto& p : panels_) clamp(m, p);
    for (auto& pair : selected_) {
        const auto ids = group(m, pair.first);
        if (std::find(ids.begin(), ids.end(), pair.second) == ids.end())
            pair.second = ids.empty() ? 0 : ids.front();
    }
}
bool Presentation::open(const Model& m, Id id, unsigned rows) {
    if (!current(m) || !accessible(m, id) || rows == 0 || rows > 64) return false;
    for (auto& p : panels_) if (p.container == id) return true;
    if (panels_.size() >= 64) return false;
    panels_.push_back({id, false, 0, rows});
    return true;
}
bool Presentation::close(Id id) {
    const auto it = std::find_if(panels_.begin(), panels_.end(), [id](const PanelState& p) { return p.container == id; });
    if (it == panels_.end()) return false;
    panels_.erase(it); return true;
}
bool Presentation::collapse(Id id, bool value) {
    for (auto& p : panels_) if (p.container == id) { p.collapsed = value; return true; }
    return false;
}
bool Presentation::scroll(const Model& m, Id id, int rows) {
    if (!current(m)) return false;
    for (auto& p : panels_) if (p.container == id) {
        const auto next = static_cast<std::int64_t>(p.first_row) + rows;
        p.first_row = static_cast<unsigned>(std::max<std::int64_t>(0, std::min<std::int64_t>(next, std::numeric_limits<unsigned>::max())));
        clamp(m, p); return true;
    }
    return false;
}
bool Presentation::viewport(const Model& m, Id id, unsigned rows) {
    if (!current(m) || rows == 0 || rows > 64) return false;
    for (auto& p : panels_) if (p.container == id) { p.visible_rows = rows; clamp(m, p); return true; }
    return false;
}
Id Presentation::cycle(const Model& m, unsigned value, int direction) {
    if (!current(m)) return 0;
    const auto ids = group(m, value);
    if (ids.empty()) return selected_[value] = 0;
    const auto it = std::find(ids.begin(), ids.end(), selected(value));
    if (it == ids.end()) return selected_[value] = ids.front();
    const auto index = static_cast<std::size_t>(it - ids.begin());
    return selected_[value] = ids[(index + (direction < 0 ? ids.size() - 1 : direction > 0 ? 1 : 0)) % ids.size()];
}
Id Presentation::selected(unsigned value) const {
    const auto it = selected_.find(value);
    return it == selected_.end() ? 0 : it->second;
}
std::vector<PanelState> Presentation::panels(const Model& m) const {
    return current(m) ? panels_ : std::vector<PanelState>{};
}
OverflowLayout Presentation::overflow_layout(const Model& m,Id id) const {
    OverflowLayout layout;const auto items=overflow_items(m,id);if(items.empty())return layout;
    layout.columns=m.nodes().at(id).grid.w;
    for(const auto& item:items)layout.columns=std::max(layout.columns,item.footprint.w);
    int x=0,y=0,height=0;
    for(const auto& item:items) {
        if(x+item.footprint.w>layout.columns){x=0;y+=height;height=0;}
        layout.tiles.push_back({item,{x,y,false}});x+=item.footprint.w;height=std::max(height,item.footprint.h);
    }
    layout.rows=y+height;return layout;
}
std::vector<ItemView> Presentation::overflow_items(const Model& m,Id id) const {
    std::vector<ItemView> result;
    if(!current(m)||!accessible(m,id))return result;
    for(const auto& panel:panels_)if(panel.container==id&&!panel.collapsed) {
        for(const auto& item:m.snapshot())if(item.parent==id&&item.location==Location::Overflow)result.push_back(item);
        break;
    }
    return result;
}
std::vector<ItemView> Presentation::visible_items(const Model& m, Id id) const {
    std::vector<ItemView> result;
    if (!current(m) || !accessible(m, id)) return result;
    for (const auto& p : panels_) if (p.container == id && !p.collapsed) {
        for (const auto& item : m.snapshot()) {
            if (item.parent == id && (carried(m, id) || world_.count(item.id) != 0) && item.location == Location::Grid &&
                item.position.y < static_cast<int>(p.first_row + p.visible_rows) &&
                item.position.y + item.footprint.h > static_cast<int>(p.first_row)) result.push_back(item);
        }
        break;
    }
    return result;
}
} // namespace inventory_next
