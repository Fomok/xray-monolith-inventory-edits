add_module(XRay.Particles
  ROOT
  
  SOURCES
  psystem.h
  stdafx.h

  INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
)

include(XRay.Particles.API)