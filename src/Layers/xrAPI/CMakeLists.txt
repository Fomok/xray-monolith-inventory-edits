add_library(xrAPI STATIC)

set_property(
  TARGET xrAPI
  PROPERTY FOLDER
  ${FOLDER_XRAY}
)

target_sources_grouped(
  TARGET xrAPI
  NAME "Kernel/Source Files"
  FILES
  stdafx.cpp
  xrAPI.cpp
)

target_sources_grouped(
  TARGET xrAPI
  NAME "Kernel/Header Files"
  FILES
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
