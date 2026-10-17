#ifndef SCOPE_GLASS_NVG_FOCUS_H
#define SCOPE_GLASS_NVG_FOCUS_H
Texture2D scope_glass_nvg_motion;
float4 scope_glass_nvg_focus;

float scope_glass_nvg_depth(int2 pixel)
{
    // NVG runs at display resolution after reconstruction; lens tags and depth
    // still belong to the reduced main scene. Convert the pixel before loading.
    uint width, height;
    s_position.GetDimensions(width, height);
    pixel = clamp(int2((float2(pixel) + 0.5) * float2(width, height) / screen_res_real.xy),
        int2(0, 0), int2(width, height) - 1);
    // Lens pixels that show the optical image focus at infinity, as the stock
    // PiP does through its zero depth stamp.
    float depth = s_position.Load(int3(pixel, 0), 0).z;
    if (scope_glass_nvg_focus.x > 0.5)
    {
        float tag = scope_glass_nvg_motion.Load(int3(pixel, 0)).w;
        if (tag > 2.5 && tag < 3.5) depth = 10000.0;
    }
    return depth;
}
#endif
