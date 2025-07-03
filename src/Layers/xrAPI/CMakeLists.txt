add_module(XRay.Render.API
  INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
  PRECOMPILES stdafx.h
  LINKS XRay.Render
  SOURCES
  xrAPI.cpp
  stdafx.h
  xrAPI.h
)
