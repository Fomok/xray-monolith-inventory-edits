add_library(cs INTERFACE)
target_folder(cs ${FOLDER_SDK})

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

target_include_directories(cs
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
)
