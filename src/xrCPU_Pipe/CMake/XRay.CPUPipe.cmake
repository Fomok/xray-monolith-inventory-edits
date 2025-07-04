add_module(XRay.CPUPipe
  ROOT

  INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}

  PRECOMPILES stdafx.h

  LINKS
  XRay.Render.API
  XRay.Collision
  XRay.Render

  SOURCES
  xrCPU_Pipe.cpp
  xrCPU_Pipe.h
)

add_module(XRay.CPUPipe.PLC
  SOURCES
  PLC.cpp
)

add_module(XRay.CPUPipe.Resources
  SOURCES
  resource.h
  xrCPU_Pipe.rc
)

add_module(XRay.CPUPipe.Skinning
  SOURCES
  xrSkin2W.cpp
  xrSkin2W_SSE.cpp
  xrSkin2W_thread.cpp
)

add_module(XRay.CPUPipe.TTAPI
  SOURCES
  ttapi.cpp
  ttapi.h
)