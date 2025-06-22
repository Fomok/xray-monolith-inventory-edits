add_custom_target(Shaders)

set_property(
  TARGET Shaders
  PROPERTY FOLDER
  ${FOLDER_GAMEDATA}
)

target_sources_grouped(
  TARGET Shaders
  NAME R2
  FILES
  r2/combine_2_naa.ps
  r2/fakescope.ps
  r2/heatvision.ps
)

target_sources_grouped(
  TARGET Shaders
  NAME R3
  FILES
  r3/combine_2_naa.ps
  r3/common_functions.h
  r3/deffer_tree_branch_bump_d-hq.vs
  r3/deffer_tree_branch_bump-hq.vs
  r3/deffer_tree_branch_flat.vs
  r3/editor.vs
  r3/effects_sun.ps
  r3/effects_sun.s
  r3/fakescope.ps
  r3/hdr10.h
  r3/hdr10_bloom.h
  r3/hdr10_bloom_blur.ps
  r3/hdr10_bloom_downsample.ps
  r3/hdr10_bloom_upsample.ps
  r3/hdr10_lens_flare.h
  r3/hdr10_lens_flare_blur.ps
  r3/hdr10_lens_flare_downsample.ps
  r3/hdr10_lens_flare_fgen.ps
  r3/hdr10_lens_flare_upsample.ps
  r3/hud_cursor.s
  r3/hud_default.ps
  r3/hud_default.s
  r3/hud_font.ps
  r3/models_selflight_det.s
  r3/night_vision.h
  r3/postprocess.ps
  r3/postprocess_cm.ps
  r3/simple_color.ps
  r3/yuv2rgb.ps
)
