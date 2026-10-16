#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace Bodycam
{
// Bounded first-order angular follower. No spring velocity or overshoot.
// Integrates a linear input ramp exactly, making ordinary mouse motion
// independent of render frequency. Angles and limits are in radians.
struct WeaponFollowState
{
    float yaw = 0.f, pitch = 0.f;
    float previous_yaw = 0.f, previous_pitch = 0.f;
    std::uint32_t last_frame = 0, last_item = 0;
    bool initialized = false;

    static float Wrap(float value)
    {
        return std::remainder(value, 6.28318530718f);
    }

    void Update(float aim_yaw, float aim_pitch, float dt, std::uint32_t frame,
        std::uint32_t item, bool enabled, float tau, float yaw_limit, float pitch_limit)
    {
        if (!std::isfinite(aim_yaw) || !std::isfinite(aim_pitch) || !std::isfinite(dt) ||
            !std::isfinite(tau) || !std::isfinite(yaw_limit) || !std::isfinite(pitch_limit))
        {
            *this = WeaponFollowState{};
            return;
        }
        const bool reset = !initialized || !enabled || item != last_item ||
            (frame != last_frame && frame != last_frame + 1) || dt > .25f || dt < 0.f;
        if (reset)
        {
            yaw = pitch = 0.f;
            previous_yaw = aim_yaw;
            previous_pitch = aim_pitch;
            last_frame = frame;
            last_item = item;
            initialized = enabled;
            return;
        }
        if (frame == last_frame || dt == 0.f)
            return;
        last_frame = frame;
        const float dy = Wrap(aim_yaw - previous_yaw);
        const float dp = Wrap(aim_pitch - previous_pitch);
        previous_yaw = aim_yaw;
        previous_pitch = aim_pitch;
        if (tau <= .0001f || std::abs(dy) > 1.f || std::abs(dp) > 1.f)
        {
            yaw = pitch = 0.f;
            return;
        }
        const float response = -std::expm1(-dt / tau);
        const float ramp = tau * response / dt;
        yaw_limit = std::max(0.f, yaw_limit);
        pitch_limit = std::max(0.f, pitch_limit);
        yaw = std::clamp(yaw * (1.f - response) - dy * ramp, -yaw_limit, yaw_limit);
        pitch = std::clamp(pitch * (1.f - response) - dp * ramp, -pitch_limit, pitch_limit);
    }
};
}
