add_library(xrParticles STATIC)

target_sources(xrParticles PRIVATE
    noise.cpp
    particle_actions.cpp
    particle_actions_collection.cpp
    particle_actions_collection_io.cpp
    particle_core.cpp
    particle_effect.cpp
    particle_manager.cpp
    stdafx.cpp
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
  PUBLIC
  NDEBUG
  XR_PARTICLES_EXPORTS

  PRIVATE
  WIN32
  _WINDOWS
  _SECURE_SCL=0
)

enable_parallel_build(xrParticles)
disable_exceptions()
silence_stdext_hash_deprecation_warnings(xrParticles)
