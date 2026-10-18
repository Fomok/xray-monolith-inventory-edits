#pragma once
namespace personal_fg
{
// Console switches enable scope rendering globally; they do not mean a scope
// is currently visible. Match the fake-scope shader's aim/radius gate, and
// conservatively suspend all aim transitions with the 3D scope feature enabled.
inline bool ScopeActive(bool svp, bool fake2d, bool fake3d, float aim, float radius)
{
    return svp || (aim > 0.f && (fake3d || (fake2d && radius > 0.f)));
}
}
