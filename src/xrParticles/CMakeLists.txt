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
  
  noise.cpp
  particle_actions.cpp
  particle_actions_collection.cpp
  particle_actions_collection_io.cpp
  particle_core.cpp
  particle_effect.cpp
  particle_manager.cpp
  
  noise.h
  particle_actions.h
  particle_actions_collection.h
  particle_core.h
  particle_effect.h
  particle_manager.h
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
)
