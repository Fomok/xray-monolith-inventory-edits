add_library(xrParticles STATIC)

set_property(
  TARGET xrParticles
  PROPERTY FOLDER
  ${FOLDER_XRAY}
)

target_sources_grouped(
  TARGET xrParticles
  NAME "Kernel"
  FILES
  psystem.h
  stdafx.h
)

target_sources_grouped(
  TARGET xrParticles
  NAME "PAPI/Source Files"
  FILES
  noise.cpp
  particle_actions.cpp
  particle_actions_collection.cpp
  particle_actions_collection_io.cpp
  particle_core.cpp
  particle_effect.cpp
  particle_manager.cpp
)

target_sources_grouped(
  TARGET xrParticles
  NAME "PAPI/Header Files"
  FILES
  noise.h
  particle_actions.h
  particle_actions_collection.h
  particle_core.h
  particle_effect.h
  particle_manager.h
)

target_include_directories(xrParticles
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_precompile_headers(xrParticles
  PRIVATE stdafx.h
)

target_link_libraries(xrParticles
  PUBLIC
  tbb
  xrCore
  xrCPU_Pipe
)
