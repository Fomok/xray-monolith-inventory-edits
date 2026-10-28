#pragma once
#include "model.h"
#include <limits>

namespace inventory_next {
using EquipmentSlotId=std::uint32_t;
struct EquipmentSlotDefinition {
    EquipmentSlotId id=0;
    std::string key,families,label;
    unsigned model_bit=0,minimum_condition=0;
    bool mounted=false,enabled=true;
    bool accepts(const std::string& family) const {
        return ("|"+families+"|").find("|"+family+"|")!=std::string::npos;
    }
};
// The entire u16 vanilla API domain is reserved, including presently unused
// slot IDs. These u32 IDs belong to an independent native inventory domain.
class EquipmentRegistry {
public:
    Result add(EquipmentSlotDefinition definition) {
        if(definition.id<=std::numeric_limits<std::uint16_t>::max()||definition.id==std::numeric_limits<EquipmentSlotId>::max())return {false,"reserved_equipment_slot"};
        if(definition.key.empty()||definition.key.size()>96||definition.families.empty()||definition.minimum_condition>10000)return {false,"equipment_slot_definition"};
        if(!definition.mounted&&(definition.model_bit<19||definition.model_bit>=48))return {false,"equipment_model_slot"};
        if(definition.mounted&&definition.model_bit)return {false,"equipment_model_slot"};
        for(const auto& pair:slots_)if(pair.first==definition.id||pair.second.key==definition.key||
            (!definition.mounted&&!pair.second.mounted&&pair.second.model_bit==definition.model_bit))return {false,"equipment_slot_collision"};
        slots_.emplace(definition.id,std::move(definition));return {true,{}};
    }
    const EquipmentSlotDefinition* find(EquipmentSlotId id) const {auto it=slots_.find(id);return it==slots_.end()?nullptr:&it->second;}
    const EquipmentSlotDefinition* find(const std::string& key) const {for(const auto& entry:slots_)if(entry.second.key==key)return &entry.second;return nullptr;}
    const std::map<EquipmentSlotId,EquipmentSlotDefinition>& slots() const {return slots_;}
private:
    std::map<EquipmentSlotId,EquipmentSlotDefinition> slots_;
};
}
