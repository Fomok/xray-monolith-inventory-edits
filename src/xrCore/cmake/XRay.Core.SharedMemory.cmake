add_library(XRay.Core.SharedMemory STATIC)

set_property(
  TARGET XRay.Core.SharedMemory
  PROPERTY FOLDER
  ${FOLDER_XRAY_CORE}
)

target_sources(XRay.Core.SharedMemory
  PRIVATE
  crc32.cpp
  mezz_stringbuffer.cpp
  xr_shared.cpp
  xrsharedmem.cpp
  xrstring.cpp
  
  mezz_stringbuffer.h
  xr_resource.h
  xr_shared.h
  xrsharedmem.h
  xrstring.h
)

target_include_directories(XRay.Core.SharedMemory
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core.SharedMemory
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core.SharedMemory
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core.SharedMemory
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.CDB
)
