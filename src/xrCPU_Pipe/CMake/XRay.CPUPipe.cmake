add_module(XRay.CPUPipe
  INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}

  PRECOMPILES stdafx.h

  LINKS
  XRay.Render.API
  XRay.Collision
  XRay.Render

  SOURCES
  xrCPU_Pipe.cpp
  
  StdAfx.h
  xrCPU_Pipe.h
  
  PLC.cpp
  
  resource.h
  xrCPU_Pipe.rc
  
  xrSkin2W.cpp
  xrSkin2W_SSE.cpp
  xrSkin2W_thread.cpp
  
  ttapi.cpp
  
  ttapi.h
)