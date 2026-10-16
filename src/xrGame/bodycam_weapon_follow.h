#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace Bodycam
{
// Bounded first-order angular follower. No spring velocity or overshoot.
// Linear response in the centre, progressively increasing resistance near
// each limit. Short integration steps keep the soft edge stable across frame
// rates. Angles and limits are in radians.
struct WeaponFollowState
{
    float yaw = 0.f, pitch = 0.f;
    float return_strength = 1.f;
    float previous_yaw = 0.f, previous_pitch = 0.f;
    std::uint32_t last_frame = 0, last_item = 0;
    bool initialized = false;

    static float Wrap(float value)
    {
        return std::remainder(value, 6.28318530718f);
    }

    // Exact outward push through a soft boundary: resistance starts at one
    // at the knee and falls quadratically towards zero at the limit. Inward
    // motion is unrestricted, so reversing does not have to unwind hidden lag.
    static float Push(float value, float delta, float limit, float softness)
    {
        if (limit <= .000001f) return 0.f;
        if (softness <= .0001f) return std::clamp(value + delta, -limit, limit);
        const float direction = delta < 0.f ? -1.f : 1.f;
        float x = std::clamp(value * direction, -limit, limit);
        float push = std::abs(delta);
        const float width = limit * softness;
        const float knee = limit - width;
        if (x < knee)
        {
            const float linear = std::min(push, knee - x);
            x += linear;
            push -= linear;
        }
        if (push > 0.f)
        {
            const float distance = std::max(0.f, limit - x);
            x = limit - distance / (1.f + push * distance / (width * width));
        }
        return direction * x;
    }

    static float Step(float value, float delta, float dt, float tau, float limit, float softness, float recovery = 1.f)
    {
        if (limit <= .000001f) return 0.f;
        value = std::clamp(value, -limit, limit);
        // No soft-edge penalty in the centre: this reduces to the original
        // exact linear ramp integration there. Bound work to 60 steps at 4 FPS.
        const int steps = static_cast<int>(std::ceil(dt * 240.f));
        const float h = dt / steps;
        const float exponent = h * recovery / tau;
        const float response = -std::expm1(-exponent);
        const float ramp = exponent > .000001f ? response / exponent : 1.f;
        const float push = -delta * ramp / steps;
        for (int i = 0; i < steps; ++i)
            value = Push(value * (1.f - response), push, limit, softness);
        return value;
    }

    void Update(float aim_yaw, float aim_pitch, float dt, std::uint32_t frame,
        std::uint32_t item, bool enabled, float tau, float yaw_limit, float pitch_limit, float softness = .5f, bool standing_hold = false)
    {
        if (!std::isfinite(aim_yaw) || !std::isfinite(aim_pitch) || !std::isfinite(dt) ||
            !std::isfinite(softness) || !std::isfinite(tau) || !std::isfinite(yaw_limit) || !std::isfinite(pitch_limit))
        {
            *this = WeaponFollowState{};
            return;
        }
        const bool reset = !initialized || !enabled || item != last_item ||
            (frame != last_frame && frame != last_frame + 1) || dt > .25f || dt < 0.f;
        if (reset)
        {
            yaw = pitch = 0.f;
            return_strength = standing_hold ? 0.f : 1.f;
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
        // Blend the return force, never the accumulated angle. This keeps
        // walking/standing transitions continuous without recentring jumps.
        const float target = standing_hold ? 0.f : 1.f;
        const float old_strength = return_strength;
        return_strength += std::clamp(target - return_strength, -dt / .15f, dt / .15f);
        const float recovery = (old_strength + return_strength) * .5f;
        softness = std::clamp(softness, 0.f, 1.f);
        yaw = Step(yaw, dy, dt, tau, yaw_limit, softness, recovery);
        pitch = Step(pitch, dp, dt, tau, pitch_limit, softness, recovery);
    }
};
}
