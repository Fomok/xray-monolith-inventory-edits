include(cmake/XRay.Core.Compression.LZ.cmake)
include(cmake/XRay.Core.Compression.LZO.cmake)
include(cmake/XRay.Core.Compression.PPMD.cmake)
include(cmake/XRay.Core.Compression.RT.cmake)
include(cmake/XRay.Core.Debug.cmake)
include(cmake/XRay.Core.Debug.BlackBox.cmake)
include(cmake/XRay.Core.FS.cmake)
include(cmake/XRay.Core.IntrusivePointer.cmake)
include(cmake/XRay.Core.Math.cmake)
include(cmake/XRay.Core.MemoryManager.cmake)
include(cmake/XRay.Core.MemoryMonitor.cmake)
include(cmake/XRay.Core.OS.cmake)
include(cmake/XRay.Core.Profiling.cmake)
include(cmake/XRay.Core.SharedMemory.cmake)
include(cmake/XRay.Core.Threading.cmake)

add_library(XRay.Core STATIC)

set_property(
  TARGET XRay.Core
  PROPERTY FOLDER
  ${FOLDER_XRAY_CORE}
)

target_sources(XRay.Core
  PRIVATE
  FTimer.cpp
  xrCore.cpp
  
  FTimer.h
  stdafx.h
  xrCore.h
  xrCore_platform.h
  
  ../build_config_defines.h
  ChooseTypes.H
  client_id.h
  robin_hood.h

  resource.h
  xrCore.rc
)

target_include_directories(XRay.Core
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.Collision
  XRay.Core.Compression.LZ
  XRay.Core.Compression.LZO
  XRay.Core.Compression.PPMD
  XRay.Core.Compression.RT
  XRay.Core.Debug
  XRay.Core.Debug.BlackBox
  XRay.Core.FS
  XRay.Core.IntrusivePointer
  XRay.Core.Math
  XRay.Core.MemoryManager
  XRay.Core.MemoryMonitor
  XRay.Core.OS
  XRay.Core.Profiling
  XRay.Core.SharedMemory
  XRay.Core.Threading
)
