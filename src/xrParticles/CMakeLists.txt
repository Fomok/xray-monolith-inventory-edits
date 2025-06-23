add_library(xrParticles STATIC)

set_property(
  TARGET xrParticles
  PROPERTY FOLDER
  ${FOLDER_XRAY}
)

target_sources_grouped(
  TARGET xrParticles
  NAME "Kernel/Source Files"
  FILES
  stdafx.cpp
)

target_sources_grouped(
  TARGET xrParticles
  NAME "Kernel/Header Files"
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
  "${CMAKE_SOURCE_DIR}/src/3rd party"
)

target_precompile_headers(xrParticles
  PRIVATE stdafx.h
)

target_compile_definitions(xrParticles
  PRIVATE
  XR_PARTICLES_EXPORTS
)

enable_parallel_build(xrParticles)
silence_stdext_hash_deprecation_warnings(xrParticles)
