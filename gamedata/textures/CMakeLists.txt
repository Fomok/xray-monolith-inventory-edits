add_custom_target(Textures)

set_property(
  TARGET Textures
  PROPERTY FOLDER
  ${FOLDER_GAMEDATA}
)

target_sources_grouped(
  TARGET Textures
  NAME UI
  FILES
  ui/cursor_dot.dds
  ui/cursor_cross.dds
  ui/cursor_plus.dds
)
