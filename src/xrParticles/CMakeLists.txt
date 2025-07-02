include(cmake/XRay.Particles.API.cmake)

add_library(XRay.Particles INTERFACE)

target_folder(XRay.Particles ${FOLDER_XRAY_PARTICLES})

target_sources(XRay.Particles
  INTERFACE
  psystem.h
  stdafx.h
)

target_link_libraries(XRay.Particles
  INTERFACE
  XRay.Particles.API
)
