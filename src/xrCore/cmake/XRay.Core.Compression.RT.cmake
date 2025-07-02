add_library(XRay.Core.Compression.RT STATIC)

target_folder(XRay.Core.Compression.RT ${FOLDER_XRAY_CORE})

target_sources(XRay.Core.Compression.RT
  PRIVATE
  rt_compressor.cpp
  rt_compressor9.cpp
  
  rt_compressor.h
)

target_include_directories(XRay.Core.Compression.RT
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core.Compression.RT
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core.Compression.RT
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core.Compression.RT
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.Collision
)
