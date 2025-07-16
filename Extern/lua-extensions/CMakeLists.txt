add_library(lua-extensions STATIC)

target_folder(lua-extensions ${FOLDER_LUA})

target_sources(lua-extensions PRIVATE
  base.cpp
  script_additional_libs.cpp
  script_additional_libs.h
)

target_include_directories(lua-extensions
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
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
