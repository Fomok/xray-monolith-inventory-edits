#pragma once
namespace personal_fg
{
// Global scope switches are not evidence of a live optical image. At hip a
// physical glass surface follows the main camera; an aimed lens needs PiP inputs.
inline bool ScopeActive(bool svp, bool fake2d, bool visibleLens, float aim, float radius)
{
    return svp || (aim > 0.f && (visibleLens || (fake2d && radius > 0.f)));
}
}
