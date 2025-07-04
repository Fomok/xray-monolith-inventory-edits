add_module(XRay.Render.DX9
  TYPE CUSTOM
  INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
  SOURCES
  dx9R_Backend_Runtime.h
  dx9r_constants_cache.cpp
  dx9r_constants_cache.h
)
