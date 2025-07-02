include(cmake/XRay.Collision.Engine.cmake)
include(cmake/XRay.Collision.Opcode.cmake)

add_library(XRay.Collision STATIC)

target_folder(XRay.Collision ${FOLDER_XRAY_COLLISION})

target_sources(XRay.Collision
  PRIVATE
  #cl_raypick.cpp
  Frustum.cpp
  xrCDB.cpp
  xrCDB_box.cpp
  xrCDB_Collector.cpp
  xrCDB_frustum.cpp
  xrCDB_ray.cpp

  Frustum.h
  StdAfx.h
  xrCDB.h
)

target_include_directories(XRay.Collision
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Collision
  PRIVATE stdafx.h
)

target_compile_definitions(XRay.Collision
  PRIVATE
  XRCDB_EXPORTS
)

target_link_libraries(XRay.Collision
  PRIVATE
  XRay.Core
  XRay.Collision.Engine
  XRay.Collision.Opcode
  XRay.Render
  XRay.Render.API
)
