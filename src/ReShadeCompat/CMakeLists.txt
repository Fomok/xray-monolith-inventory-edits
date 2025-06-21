add_library(ReShadeCompat STATIC)

target_sources(ReShadeCompat PRIVATE
  addon.cpp
  stdafx.cpp
)

target_include_directories(ReShadeCompat
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/src/3rd party"
)

target_compile_definitions(ReShadeCompat
  PUBLIC
  NDEBUG
  PURE_ALLOC
  XRCORE_EXPORTS

  PRIVATE
  WIN32
  _WINDOWS
  _USRDLL
  _SECURE_SCL=0
)

target_precompile_headers(ReShadeCompat
  PRIVATE stdafx.h
)

enable_parallel_build(ReShadeCompat)
