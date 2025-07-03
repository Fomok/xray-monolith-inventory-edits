add_module(XRay.Core
  PRECOMPILES stdafx.h

  DEFINES
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL

  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}

  LINKS
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.Collision

  SOURCES
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