add_library(XRay.Core.MemoryMonitor STATIC)

target_folder(XRay.Core.MemoryMonitor ${FOLDER_XRAY_CORE})

target_sources(XRay.Core.MemoryMonitor
  PRIVATE
  memory_monitor.cpp
  
  memory_monitor.h
)

target_include_directories(XRay.Core.MemoryMonitor
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core.MemoryMonitor
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core.MemoryMonitor
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core.MemoryMonitor
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.Collision
)
