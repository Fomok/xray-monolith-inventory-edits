#pragma once
#include "model.h"

namespace inventory_next {
// Possession remains a separate query. Quick access reaches an actual grid
// member of the worn rig or one active mounted pouch, never a closed box/spare.
inline bool equipped_rig_access(const Model& model,Id item) {
    auto member=model.nodes().find(item);
    if(member==model.nodes().end()||member->second.kind!=Kind::Item||member->second.location!=Location::Grid)return false;
    auto owner=model.nodes().find(member->second.parent);
    if(owner==model.nodes().end())return false;
    if(owner->second.location==Location::Mounted) {
        if(!owner->second.pouch||!model.storage_active(owner->first))return false;
        owner=model.nodes().find(owner->second.parent);if(owner==model.nodes().end())return false;
    }
    const auto family=owner->second.attributes.find("storage_family");
    return owner->second.parent==1&&owner->second.location==Location::Equipment&&
        family!=owner->second.attributes.end()&&family->second=="rig";
}
}
