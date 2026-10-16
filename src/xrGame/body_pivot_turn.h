#pragma once
#include <algorithm>
#include <cmath>

namespace body_pivot
{
constexpr float pi = 3.14159265358979323846f;
inline float radians(float degrees) { return degrees * pi / 180.f; }
inline float wrap(float angle) { return std::remainder(angle, 2.f * pi); }
struct Turn { float yaw; bool turning; };
// Respond immediately to collision pressure; ease back only when space opens.
inline float retreat(float current, float required, float dt)
{
    if (!std::isfinite(current)) current = 0.f;
    if (!std::isfinite(required)) required = 0.f;
    required = std::clamp(required, 0.f, 2.5f);
    if (required >= current) return required;
    if (!std::isfinite(dt) || dt <= 0.f) return current;
    const float next = required + (current - required) * std::exp(-12.f * std::min(dt, 0.1f));
    return next < 0.001f ? 0.f : next;
}
// Uses native model yaw so existing spine/head and turn animations agree.
inline Turn step(float yaw, float aim, float dt, bool moving, bool turning)
{
    if (!std::isfinite(yaw) || !std::isfinite(aim) || !std::isfinite(dt))
        return {std::isfinite(aim) ? wrap(aim) : 0.f, false};
    if (dt <= 0.f) return {yaw, turning && !moving};
    dt = std::min(dt, 0.1f);
    float error = wrap(aim - yaw);
    if (moving)
    {
        // Follow the look while walking, without changing the control heading.
        const float travel = radians(360.f) * dt;
        yaw += std::clamp(error * (1.f - std::exp(-12.f * dt)), -travel, travel);
        return {wrap(yaw), false};
    }
    if (std::abs(error) > radians(25.f)) turning = true;
    if (turning)
    {
        const float travel = radians(180.f) * dt;
        yaw += std::clamp(error, -travel, travel);
        error = wrap(aim - yaw);
        if (std::abs(error) <= radians(5.f)) turning = false;
    }
    return {wrap(yaw), turning};
}
}
