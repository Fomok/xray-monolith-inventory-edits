add_library(lua-extensions STATIC)

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

target_compile_definitions(lua-extensions
  PRIVATE
  _WINDLL
  _CRT_SECURE_NO_WARNINGS
)

enable_parallel_build(lua-extensions)
