add_library(XRay.Core.Compression.PPMD STATIC)

set_property(
  TARGET XRay.Core.Compression.PPMD
  PROPERTY FOLDER
  ${FOLDER_XRAY_CORE}
)

target_sources(XRay.Core.Compression.PPMD
  PRIVATE
  PPMd.h
  PPMdType.h
  
  Model.cpp
  Coder.hpp
  
  SubAlloc.hpp
  
  compression_ppmd_stream.h
  compression_ppmd_stream_inline.h
  
  ppmd_compressor.cpp
  ppmd_compressor.h
)

target_include_directories(XRay.Core.Compression.PPMD
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core.Compression.PPMD
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core.Compression.PPMD
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core.Compression.PPMD
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.CDB
)
