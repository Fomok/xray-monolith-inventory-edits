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
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/src/3rd party/luajit-2/src"
)

enable_parallel_build(lua-extensions)
