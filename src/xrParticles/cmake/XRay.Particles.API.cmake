add_library(XRay.Particles.API STATIC)

target_folder(XRay.Particles.API ${FOLDER_XRAY_PARTICLES})

target_sources(XRay.Particles.API
  PRIVATE
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

target_include_directories(XRay.Particles.API
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Particles.API
  PRIVATE stdafx.h
)

target_link_libraries(XRay.Particles.API
  PRIVATE
  tbb
  XRay.Core
  XRay.CPUPipe
)
