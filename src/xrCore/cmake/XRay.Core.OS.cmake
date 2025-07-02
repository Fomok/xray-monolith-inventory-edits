add_library(XRay.Core.OS STATIC)

set_property(
  TARGET XRay.Core.OS
  PROPERTY FOLDER
  ${FOLDER_XRAY_CORE}
)

target_sources(XRay.Core.OS
  PRIVATE
  os_clipboard.cpp
  
  os_clipboard.h
)

target_include_directories(XRay.Core.OS
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core.OS
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core.OS
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core.OS
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.Collision
)
