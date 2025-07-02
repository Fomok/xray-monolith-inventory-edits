add_custom_target(XRay.Render.DX9)

target_folder(XRay.Render.DX9 ${FOLDER_RENDER})

target_sources(XRay.Render.DX9
  PRIVATE
  dx9R_Backend_Runtime.h
  dx9r_constants_cache.cpp
  dx9r_constants_cache.h
)
