add_custom_target(Gamedata)

set_property(
  TARGET Gamedata
  PROPERTY FOLDER
  ${FOLDER_XRAY}
)

cmake_path(
  CONVERT ${CMAKE_SOURCE_DIR}/gamedata
  TO_NATIVE_PATH_LIST XRAY_GAMEDATA
)
set(XRAY_GAMEDATA ${XRAY_GAMEDATA} CACHE PATH "X-Ray Gamedata")

set(XRAY_GAMEDATA_SOURCE_DB ${CMAKE_SOURCE_DIR}/gamedata.db0)
set(XRAY_GAMEDATA_BINARY_DB ${CMAKE_CURRENT_BINARY_DIR}/gamedata.db0)

add_custom_command(
  OUTPUT gamedata.db0
  COMMAND ${XRAY_COMPRESSOR}
    ${XRAY_GAMEDATA}
    -ltx build_patch.ltx
    -pack
    -strong
    -1024
    -db
  MAIN_DEPENDENCY ${XRAY_GAMEDATA}
  BYPRODUCTS ${XRAY_COMPRESSOR_LOG}
  WORKING_DIRECTORY ${XRAY_COMPRESSOR_DIR}
  VERBATIM
)

add_custom_command(
  TARGET Gamedata
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E rm -f ${XRAY_GAMEDATA_BINARY_DB}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  VERBATIM
)

add_custom_command(
  TARGET Gamedata
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E rename
    ${XRAY_GAMEDATA_SOURCE_DB} ${XRAY_GAMEDATA_BINARY_DB}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  VERBATIM
)

target_sources(Gamedata
  PRIVATE
  gamedata.db0
)

target_sources_grouped(
  TARGET Gamedata
  NAME Configs/Unlocalizers
  FILES
  configs/unlocalizers/unlocalizer_modded_exes_axr_trade_manager.ltx
  configs/unlocalizers/unlocalizer_modded_exes_global.ltx
  configs/unlocalizers/unlocalizer_modded_exes_item_device_script_fixes.ltx
  configs/unlocalizers/unlocalizer_modded_exes_item_weapon.ltx
  configs/unlocalizers/unlocalizer_modded_exes_pda.ltx
  configs/unlocalizers/unlocalizer_modded_exes_utils_item.ltx
  configs/unlocalizers/unlocalizer_modded_exes_xrs_facer_script_fixes.ltx
  configs/unlocalizers/unlocalizer_test.ltx
  configs/unlocalizers/unlocalizer_ui_options.ltx
)

target_sources_grouped(
  TARGET Gamedata
  NAME Configs/UI
  FILES
  configs/ui/ui_ctrl_hdr.xml
  configs/ui/ui_ctrl_hdr_16.xml
)

target_sources_grouped(
  TARGET Gamedata
  NAME Configs/Text/English
  FILES
  configs/text/eng/ui_mm_modded_exes.xml
)

target_sources_grouped(
  TARGET Gamedata
  NAME Configs/Text/Russian
  FILES
  configs/text/rus/ui_mm_modded_exes.xml
)

target_sources_grouped(
  TARGET Gamedata
  NAME Configs
  FILES
  configs/mod_script_dxml.ltx
  configs/mod_system_base_hud.ltx
  configs/mod_system_flashlight_definition.ltx
  configs/mod_system_knife.ltx
  configs/mod_system_shader_crosshair.ltx
  configs/patrol_paths.ltx
)

target_sources_grouped(
  TARGET Gamedata
  NAME Materials
  FILES
  materials/material_pairs.ltx
  materials/materials.ltx
)

target_sources_grouped(
  TARGET Gamedata
  NAME Scripts
  FILES
  scripts/_g_patches.script
  scripts/aaa_sound_object_patch.script
  scripts/aaaa_script_fixes_mp.script
  scripts/axr_beh_patches.script
  scripts/callbacks_gameobject.script
  scripts/class_registrator_modded_exes.script
  scripts/dxml_core.script
  scripts/fakelens.script
  scripts/imgui_helper.script
  scripts/ltx_help_ex.script
  scripts/lua_help_ex.script
  scripts/lua_help_imgui.script
  scripts/modxml_inject_keybinds.script
  scripts/modxml_test.script
  scripts/options_builder.script
  scripts/options_modded_exes.script
  scripts/options_modded_exes_3d_ballistics.script
  scripts/options_modded_exes_3d_scopes.script
  scripts/options_modded_exes_aim.script
  scripts/options_modded_exes_camera.script
  scripts/options_modded_exes_control.script
  scripts/options_modded_exes_crash_saves.script
  scripts/options_modded_exes_crosshair.script
  scripts/options_modded_exes_debug.script
  scripts/options_modded_exes_doppler.script
  scripts/options_modded_exes_first_person_death.script
  scripts/options_modded_exes_gameplay.script
  scripts/options_modded_exes_hdr10.script
  scripts/options_modded_exes_keyboard.script
  scripts/options_modded_exes_logging.script
  scripts/options_modded_exes_optimizations.script
  scripts/options_modded_exes_monsters.script
  scripts/options_modded_exes_mouse.script
  scripts/options_modded_exes_particles.script
  scripts/options_modded_exes_pda.script
  scripts/options_modded_exes_saves.script
  scripts/options_modded_exes_sound.script
  scripts/options_modded_exes_ui_hud.script
  scripts/options_modded_exes_visual.script
  scripts/options_modded_exes_wallmarks.script
  scripts/scopeRadii.script
  scripts/slaxml.script
  scripts/true_first_person_death.script
  scripts/ui_options_modded_exes.script
)

target_sources_grouped(
  TARGET Gamedata
  NAME Shaders/R2
  FILES
  shaders/r2/combine_2_naa.ps
  shaders/r2/fakescope.ps
  shaders/r2/heatvision.ps
)

target_sources_grouped(
  TARGET Gamedata
  NAME Shaders/R3
  FILES
  shaders/r3/combine_2_naa.ps
  shaders/r3/common_functions.h
  shaders/r3/deffer_tree_branch_bump_d-hq.vs
  shaders/r3/deffer_tree_branch_bump-hq.vs
  shaders/r3/deffer_tree_branch_flat.vs
  shaders/r3/editor.vs
  shaders/r3/effects_sun.ps
  shaders/r3/effects_sun.s
  shaders/r3/fakescope.ps
  shaders/r3/hdr10.h
  shaders/r3/hdr10_bloom.h
  shaders/r3/hdr10_bloom_blur.ps
  shaders/r3/hdr10_bloom_downsample.ps
  shaders/r3/hdr10_bloom_upsample.ps
  shaders/r3/hdr10_lens_flare.h
  shaders/r3/hdr10_lens_flare_blur.ps
  shaders/r3/hdr10_lens_flare_downsample.ps
  shaders/r3/hdr10_lens_flare_fgen.ps
  shaders/r3/hdr10_lens_flare_upsample.ps
  shaders/r3/hud_cursor.s
  shaders/r3/hud_default.ps
  shaders/r3/hud_default.s
  shaders/r3/hud_font.ps
  shaders/r3/models_selflight_det.s
  shaders/r3/night_vision.h
  shaders/r3/postprocess.ps
  shaders/r3/postprocess_cm.ps
  shaders/r3/simple_color.ps
  shaders/r3/yuv2rgb.ps
)

target_sources_grouped(
  TARGET Gamedata
  NAME Textures/UI
  FILES
  textures/ui/cursor_dot.dds
  textures/ui/cursor_cross.dds
  textures/ui/cursor_plus.dds
)
