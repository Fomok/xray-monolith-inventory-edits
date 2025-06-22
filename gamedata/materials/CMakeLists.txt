add_custom_target(Materials)

set_property(
  TARGET Materials
  PROPERTY FOLDER
  ${FOLDER_GAMEDATA}
)

target_sources(Materials
  PRIVATE
  material_pairs.ltx
  materials.ltx
)
