#pragma once
#include "model.h"
#include <set>

namespace inventory_next {
struct PanelState {
    Id container = 0;
    bool collapsed = false;
    unsigned first_row = 0, visible_rows = 8;
};

// Read-only controller shared by a native renderer and the background lab.
// World permissions are supplied by the adapter for each specific container;
// allowing a corpse/root does not implicitly reveal nested bags or hidden loot.
class Presentation {
public:
    void synchronize(const Model& model, Id actor,
                     const std::set<Id>& visible_world_objects);
    bool open(const Model& model, Id container, unsigned visible_rows = 8);
    bool close(Id container);
    bool collapse(Id container, bool value);
    bool scroll(const Model& model, Id container, int rows);
    bool viewport(const Model& model, Id container, unsigned visible_rows);
    Id cycle(const Model& model, unsigned equipment_group, int direction);
    Id selected(unsigned equipment_group) const;
    std::vector<PanelState> panels(const Model& model) const;
    // Only geometrically visible grid children, never equipment or collapsed rows.
    std::vector<ItemView> visible_items(const Model& model, Id container) const;
private:
    bool current(const Model& model) const;
    bool accessible(const Model& model, Id container) const;
    bool carried(const Model& model, Id item) const;
    std::vector<Id> group(const Model& model, unsigned value) const;
    void clamp(const Model& model, PanelState& panel) const;
    std::uint64_t session_ = 0, revision_ = 0;
    Id actor_ = 0;
    std::set<Id> world_;
    std::vector<PanelState> panels_;
    std::map<unsigned, Id> selected_;
};
} // namespace inventory_next
