include(cmake/XRay.Particles.API.cmake)

add_library(XRay.Particles INTERFACE)

set_property(
  TARGET XRay.Particles
  PROPERTY FOLDER
  ${FOLDER_XRAY_PARTICLES}
)

target_sources(XRay.Particles
  INTERFACE
  psystem.h
  stdafx.h
)

target_link_libraries(XRay.Particles
  INTERFACE
  XRay.Particles.API
)
