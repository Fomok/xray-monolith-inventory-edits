include(cmake/XRay.CDB.Engine.cmake)
include(cmake/XRay.CDB.Opcode.cmake)

add_library(XRay.CDB STATIC)

set_property(
  TARGET XRay.CDB
  PROPERTY FOLDER
  ${FOLDER_XRAY_CDB}
)

target_sources(XRay.CDB
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

target_include_directories(XRay.CDB
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.CDB
  PRIVATE stdafx.h
)

target_compile_definitions(XRay.CDB
  PRIVATE
  XRCDB_EXPORTS
)

target_link_libraries(XRay.CDB
  PRIVATE
  XRay.Core
  XRay.CDB.Engine
  XRay.CDB.Opcode
  XRay.Render
  XRay.Render.API
)
