#pragma once
#include <algorithm>
#include <string>
#include <vector>

// SInvItemPlace stores slot identities in six bits. Zero is the inactive slot.
namespace SqaEquipmentSlots
{
constexpr unsigned Limit = 64;
inline std::vector<std::string> Allocate(unsigned legacyLast, std::vector<std::string> names)
{
    std::sort(names.begin(), names.end());
    names.erase(std::unique(names.begin(), names.end()), names.end());
    if (legacyLast >= Limit || names.size() > Limit - 1 - legacyLast)
        names.clear(); // Fail the entire registry closed; never alias another role.
    return names;
}
inline unsigned Resolve(unsigned legacyLast, const std::vector<std::string>& names, const char* role)
{
    if (!role || !*role) return 0;
    const auto found = std::lower_bound(names.begin(), names.end(), std::string(role));
    return found != names.end() && *found == role ? legacyLast + 1 + unsigned(found - names.begin()) : 0;
}
}
