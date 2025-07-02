add_library(XRay.Core.Compression.LZ STATIC)

set_property(
  TARGET XRay.Core.Compression.LZ
  PROPERTY FOLDER
  ${FOLDER_XRAY_CORE}
)

target_sources(XRay.Core.Compression.LZ
  PRIVATE
  LzHuf.cpp
  lzhuf.h
)

target_include_directories(XRay.Core.Compression.LZ
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core.Compression.LZ
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core.Compression.LZ
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core.Compression.LZ
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.CDB
)
