add_custom_target(xrRenderDX9)

set_property(
  TARGET xrRenderDX9
  PROPERTY FOLDER
  ${FOLDER_RENDER}
)

target_sources(xrRenderDX9
  PRIVATE
  dx9R_Backend_Runtime.h
  dx9r_constants_cache.cpp
  dx9r_constants_cache.h
)
