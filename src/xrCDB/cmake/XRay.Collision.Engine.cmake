add_library(XRay.Collision.Engine STATIC)

set_property(
  TARGET XRay.Collision.Engine
  PROPERTY FOLDER
  ${FOLDER_XRAY_COLLISION}
)

target_sources(XRay.Collision.Engine
  PRIVATE
  ISpatial.cpp
  ISpatial_q_box.cpp
  ISpatial_q_frustum.cpp
  ISpatial_q_ray.cpp
  ISpatial_verify.cpp
  xr_area.cpp
  xr_area_query.cpp
  xr_area_raypick.cpp
  xrXRC.cpp
  xrXRC.h

  ISpatial.h
  xr_area.h
  xr_collide_defs.h
)

target_include_directories(XRay.Collision.Engine
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Collision.Engine
  PRIVATE stdafx.h
)

target_compile_definitions(XRay.Collision.Engine
  PRIVATE
  XRCDB_EXPORTS
)

target_link_libraries(XRay.Collision.Engine
  PRIVATE
  XRay.Render.API
  XRay.Core
  XRay.Render
)
