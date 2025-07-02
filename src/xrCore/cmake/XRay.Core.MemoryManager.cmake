add_library(XRay.Core.MemoryManager STATIC)

set_property(
  TARGET XRay.Core.MemoryManager
  PROPERTY FOLDER
  ${FOLDER_XRAY_CORE}
)

target_sources(XRay.Core.MemoryManager
  PRIVATE
  memory_allocation_stats.cpp
  memory_usage.cpp
  xrMemory.cpp
  xrMemory_align.cpp
  xrMemory_debug.cpp
  xrMemory_POOL.cpp
  xrMemory_pso_Copy.cpp
  xrMemory_pso_Fill.cpp
  xrMemory_pso_Fill32.cpp
  xrMemory_subst_borland.cpp
  xrMemory_subst_msvc.cpp
  
  xrMemory.h
  xrMemory_align.h
  xrMEMORY_POOL.h
  xrMemory_pso.h
  xrMemory_pure.h
  xrMemory_subst_borland.h
  xrMemory_subst_msvc.h
)

target_include_directories(XRay.Core.MemoryManager
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core.MemoryManager
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core.MemoryManager
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core.MemoryManager
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.CDB
)
