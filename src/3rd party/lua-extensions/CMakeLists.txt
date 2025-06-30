add_library(lua-extensions STATIC)

set_property(
  TARGET lua-extensions
  PROPERTY FOLDER
  ${FOLDER_LUA}
)

target_sources(lua-extensions PRIVATE
  base.cpp
  lfs.c
  lmarshal.c
  script_additional_libs.cpp

  lfs.h
  lmarshal.h
  script_additional_libs.h
)

target_include_directories(lua-extensions
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(lua-extensions
  PRIVATE
  LuaJIT
)
