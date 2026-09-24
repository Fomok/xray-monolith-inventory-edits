# Flat inspection background

Branch: feature/ui-background, based on all-in-one-vs2022-wpo-mt.

`attachment:set_ui_background(argb, distance)` returns true when enabled.
Use a CamAttached script attachment; park its carrier geometry behind the
camera. Distance is in camera metres. Call with distance 0 to disable, or
remove the attachment when the window closes. Unsupported renderers return
false; invalid input disables a previous background and returns false.

Initial support: DX11/DX11-AVX at feature level 11.0 or higher. The hook is
opt-in and defaults off. The mod falls back to its existing background when
this method is unavailable or returns false.

The camera UI pass follows phase_combine. A full-screen colour pass uses the
same camera projection and near viewport range as camera attachment models.
Without MSAA, hardware depth testing preserves the model. With MSAA, the
shader reads the original multisampled depth, retaining sample coverage;
combine's reconstructed world-projection depth is unsuitable here. Only the
DX11 main MSAA depth target gains shader-resource access. No depth is written.
The pass bypasses scene illumination, bloom, SSR, grading, and tone mapping.
The inspected model continues to use its existing materials and lighting.

Shader sources live in gamedata/shaders/r3 and ship with the matching mod.
No build files need new C++ compilation units. Existing UI shaders and other
attachment paths are unchanged. Shader resources are released with UI geometry.

Validation completed locally: shader model 5 vertex and pixel compilation,
including USE_MSAA; LuaJIT compilation of all four mod scripts; mocked native
hook activation, missing/unsupported engine fallback, and close cleanup.
C++ build and visual testing require the GitHub build and game respectively.

In-game checks: fireplace nearby, daylight, MSAA off/on, rotate/zoom a long
weapon, swap optics, close/reopen repeatedly, verify gameplay after closing.
Known scope: this is background isolation, not an independent studio-lighting
renderer for the weapon; foreground transparent materials/AA edges need visual
checking. Keep model geometry closer than the background plane (3.2m in mod).

## Studio lighting (mod 0.2.1)

After setting a native background, call `set_ui_studio_lighting(true)` on
its carrier. Mark the private key/fill script lights with `inspection=true`.
The DX11 renderer recomputes the active mode from live attachments each frame:
removing/disabling the background restores normal rendering automatically.
Only tagged lights are captured; sun accumulation is skipped, combine uses
neutral ambient with no environment-map tint, and SSFX IL/SSR are skipped.
No weather descriptors or saved graphics settings are modified. This is a
viewport-wide inspection presentation mode, not per-object light linking.
Other materials, exposure and postprocessing still need in-game verification.
LuaJIT syntax and mocked activation/fallback/cleanup checks pass locally.
