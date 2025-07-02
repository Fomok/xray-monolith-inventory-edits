add_library(XRay.Core.Debug STATIC)

target_folder(XRay.Core.Debug ${FOLDER_XRAY_CORE})

target_sources(XRay.Core.Debug
  PRIVATE
  #xrDebug.cpp
  xrDebugNew.cpp
)

target_include_directories(XRay.Core.Debug
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core.Debug
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core.Debug
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core.Debug
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.Collision
)
