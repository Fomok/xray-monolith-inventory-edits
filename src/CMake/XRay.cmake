add_module(XRay
  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}

  SOURCES
  build_config_defines.h
)

include(XRay.Platform)
