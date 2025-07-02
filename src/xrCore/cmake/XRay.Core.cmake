add_module(XRay.Core
  PCH stdafx.h

  DEFS
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL

  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}

  LIBS
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