include(cmake/XRay.Particles.API.cmake)

add_library(XRay.Particles STATIC)

set_property(
  TARGET XRay.Particles
  PROPERTY FOLDER
  ${FOLDER_XRAY_PARTICLES}
)

target_sources(XRay.Particles
  PRIVATE
  psystem.h
  stdafx.h
)

target_include_directories(XRay.Particles
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Particles
  PRIVATE stdafx.h
)

target_link_libraries(XRay.Particles
  PRIVATE
  tbb
  XRay.Core
  XRay.CPUPipe
  XRay.Particles.API
)
