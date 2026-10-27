#pragma once
#include "model.h"
#include <set>

namespace inventory_next {
enum class DropFeedback { None, Valid, Invalid, Swap };
struct DropProposal {
    DropFeedback feedback = DropFeedback::None;
    Result result{false,"no_drag"};
    Size footprint{0,0};
    Position position;
    std::vector<Move> moves;
};
// Gesture state only. Authoritative adapter owns access, item locks and commit.
class DragSession {
public:
    Result begin(const Model&,Id item);
    bool rotate(const Model&);
    void cancel();
    DropProposal preview(const Model&,Id destination,int x,int y,bool swap_enabled,
                         const std::set<Id>& accessible_swap_items) const;
    DropProposal release(const Model&,Id destination,int x,int y,bool swap_enabled,
                         const std::set<Id>& accessible_swap_items);
    Id item() const { return item_; }
private:
    bool current(const Model&) const;
    Id item_ = 0;
    std::uint64_t session_ = 0, revision_ = 0;
    bool rotated_ = false;
};
} // namespace inventory_next
