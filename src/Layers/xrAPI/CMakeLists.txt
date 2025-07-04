add_module(XRay.Render.API
  ROOT

  INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}

  PRECOMPILES stdafx.h

  LINKS XRay.Render
  
  SOURCES
  xrAPI.cpp
  stdafx.h
  xrAPI.h
)
