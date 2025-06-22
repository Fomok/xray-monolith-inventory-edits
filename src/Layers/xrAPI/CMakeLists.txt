add_library(xrAPI STATIC)

target_sources(xrAPI PRIVATE
  stdafx.cpp
  xrAPI.cpp

  ../../Include/xrAPI/xrAPI.h
  stdafx.h
)

target_include_directories(xrAPI
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(xrAPI
  PUBLIC
  NDEBUG
  XRAPI_EXPORTS

  PRIVATE
  WIN32
  _WINDOWS
  _USRDLL
)

target_precompile_headers(xrAPI
  PRIVATE stdafx.h
)

enable_parallel_build(xrAPI)
