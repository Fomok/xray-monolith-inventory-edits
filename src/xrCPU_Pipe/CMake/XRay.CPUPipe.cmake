add_module(XRay.CPUPipe
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
  PARENT XRay.CPUPipe

  SOURCES
  PLC.cpp
)

add_module(XRay.CPUPipe.Resources
  PARENT XRay.CPUPipe

  SOURCES
  resource.h
  xrCPU_Pipe.rc
)

add_module(XRay.CPUPipe.Skinning
  PARENT XRay.CPUPipe

  SOURCES
  xrSkin2W.cpp
  xrSkin2W_SSE.cpp
  xrSkin2W_thread.cpp
)

add_module(XRay.CPUPipe.TTAPI
  PARENT XRay.CPUPipe

  SOURCES
  ttapi.cpp
  ttapi.h
)