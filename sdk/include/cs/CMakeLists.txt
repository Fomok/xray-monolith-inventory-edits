add_custom_target(cs)

set_property(
  TARGET cs
  PROPERTY FOLDER
  ${FOLDER_SDK}
)

target_sources(cs
  PRIVATE
  config.h
  defines.h
  lua_studio_backend/backend.h
  lua_studio_backend/engine.h
  lua_studio_backend/interfaces.h
  lua_studio_backend/library_linkage.h
  lua_studio_backend/world.h
)
