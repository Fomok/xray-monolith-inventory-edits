#pragma once
namespace personal_fg
{
// Console switches enable scope rendering globally; they do not mean a scope
// is currently visible. Match the fake-scope shader's aim/radius gate, and
// A global 3D-scope setting is not evidence of a visible lens (iron sights).
inline bool ScopeActive(bool svp, bool fake2d, bool visibleLens, float aim, float radius)
{
    return svp || visibleLens || (aim > 0.f && fake2d && radius > 0.f);
}
}
