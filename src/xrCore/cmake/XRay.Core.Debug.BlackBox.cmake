add_library(XRay.Core.Debug.BlackBox STATIC)

set_property(
  TARGET XRay.Core.Debug.BlackBox
  PROPERTY FOLDER
  ${FOLDER_XRAY_CORE}
)

target_sources(XRay.Core.Debug.BlackBox
  PRIVATE
  blackbox/BlackBoxUI.cpp
  blackbox/BSUFunctions.cpp
  #blackbox/DiagAssert.cpp
  blackbox/CrashHandler.cpp
  blackbox/GetLoadedModules.cpp
  blackbox/IsNT.cpp
  blackbox/NT4ProcessInfo.cpp
  blackbox/TLHELPProcessInfo.cpp
  
  blackbox/BugslayerUtil.h
  blackbox/CrashHandler.h
  blackbox/DiagAssert.h
  blackbox/Internal.h
  blackbox/stdafx_.h
  blackbox/SymbolEngine.h
  blackbox/WarningsOff.h
  blackbox/WarningsOn.h
)

target_include_directories(XRay.Core.Debug.BlackBox
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core.Debug.BlackBox
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core.Debug.BlackBox
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core.Debug.BlackBox
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.Collision
)
