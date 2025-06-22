add_custom_target(Configs)

set_property(
  TARGET Configs
  PROPERTY FOLDER
  ${FOLDER_GAMEDATA}
)

target_sources_grouped(
  TARGET Configs
  NAME Unlocalizers
  FILES
  unlocalizers/unlocalizer_modded_exes_axr_trade_manager.ltx
  unlocalizers/unlocalizer_modded_exes_global.ltx
  unlocalizers/unlocalizer_modded_exes_item_device_script_fixes.ltx
  unlocalizers/unlocalizer_modded_exes_item_weapon.ltx
  unlocalizers/unlocalizer_modded_exes_pda.ltx
  unlocalizers/unlocalizer_modded_exes_utils_item.ltx
  unlocalizers/unlocalizer_modded_exes_xrs_facer_script_fixes.ltx
  unlocalizers/unlocalizer_test.ltx
  unlocalizers/unlocalizer_ui_options.ltx
)

target_sources_grouped(
  TARGET Configs
  NAME UI
  FILES
  ui/ui_ctrl_hdr.xml
  ui/ui_ctrl_hdr_16.xml
)

target_sources_grouped(
  TARGET Configs
  NAME Text/English
  FILES
  text/eng/ui_mm_modded_exes.xml
)

target_sources_grouped(
  TARGET Configs
  NAME Text/Russian
  FILES
  text/rus/ui_mm_modded_exes.xml
)

target_sources(Configs
  PRIVATE
  mod_script_dxml.ltx
  mod_system_base_hud.ltx
  mod_system_flashlight_definition.ltx
  mod_system_knife.ltx
  mod_system_shader_crosshair.ltx
  patrol_paths.ltx
)
