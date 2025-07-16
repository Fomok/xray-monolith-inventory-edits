add_library(lua-extensions STATIC)

target_folder(lua-extensions ${FOLDER_LUA})

target_sources(lua-extensions PRIVATE
  base.cpp
  lmarshal.c
  script_additional_libs.cpp

  lmarshal.h
  script_additional_libs.h
)

target_include_directories(lua-extensions
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(lua-extensions
  PRIVATE
  luafilesystem
  LuaPanda
  luasocket
  LuaJIT
  XRay.Core
)
