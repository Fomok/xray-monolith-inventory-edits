add_library(XRay.Core.Threading STATIC)

target_folder(XRay.Core.Threading ${FOLDER_XRAY_CORE})

target_sources(XRay.Core.Threading
  PRIVATE
  Lock.cpp
  ScopeLock.cpp
  xrSyncronize.cpp
  
  Lock.hpp
  ScopeLock.hpp
  xrSyncronize.h
)

target_include_directories(XRay.Core.Threading
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core.Threading
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core.Threading
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core.Threading
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.Collision
)
