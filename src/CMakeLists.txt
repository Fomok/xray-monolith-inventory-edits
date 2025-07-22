add_module(XRay
  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}

  SOURCES
  build_config_defines.h
)

add_subdirectory("Layers")

add_subdirectory("xrCDB")
add_subdirectory("xrCore")
add_subdirectory("xrCPU_Pipe")
add_subdirectory("xrEngine")
add_subdirectory("xrGame")
add_subdirectory("xrNetServer")
add_subdirectory("xrParticles")
add_subdirectory("xrPhysics")
add_subdirectory("xrSound")
add_subdirectory("xrXMLParser")
