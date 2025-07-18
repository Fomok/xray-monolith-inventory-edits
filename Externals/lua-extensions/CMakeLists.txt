add_library(lua-extensions STATIC)

target_folder(lua-extensions ${FOLDER_LUA})

target_sources_sandboxed(lua-extensions
  DIRECTORY lua-extensions
  
  SOURCES
  base.cpp
  script_additional_libs.cpp
  script_additional_libs.h
)

target_include_directories(lua-extensions
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/lua-extensions
)

target_link_libraries(lua-extensions
  PRIVATE
  luafilesystem
  lua-marshal
  LuaPanda
  luasocket
  LuaJIT
  XRay.Core
)
