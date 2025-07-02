add_library(XRay.Render.API STATIC)

set_property(
  TARGET XRay.Render.API
  PROPERTY FOLDER
  ${FOLDER_RENDER}
)

target_sources(XRay.Render.API
  PRIVATE
  xrAPI.cpp
  stdafx.h
  xrAPI.h
)

target_include_directories(XRay.Render.API
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Render.API
  PRIVATE stdafx.h
)

target_link_libraries(XRay.Render.API
  PRIVATE
  XRay.Render
)
