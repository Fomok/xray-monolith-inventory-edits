add_library(XRay.CDB.Engine STATIC)

set_property(
  TARGET XRay.CDB.Engine
  PROPERTY FOLDER
  ${FOLDER_XRAY_CDB}
)

target_sources(XRay.CDB.Engine
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

target_include_directories(XRay.CDB.Engine
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.CDB.Engine
  PRIVATE stdafx.h
)

target_compile_definitions(XRay.CDB.Engine
  PRIVATE
  XRCDB_EXPORTS
)

target_link_libraries(XRay.CDB.Engine
  PRIVATE
  XRay.Render.API
  XRay.Core
  XRay.Render
)
