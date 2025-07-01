add_library(xrCDB.Engine STATIC)

set_property(
  TARGET xrCDB.Engine
  PROPERTY FOLDER
  ${FOLDER_XRAY_CDB}
)

target_sources(xrCDB.Engine
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

target_include_directories(xrCDB.Engine
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(xrCDB.Engine
  PRIVATE stdafx.h
)

target_compile_definitions(xrCDB.Engine
  PRIVATE
  XRCDB_EXPORTS
)

target_link_libraries(xrCDB.Engine
  PRIVATE
  xrAPI
  xrCore
  xrRender
)
