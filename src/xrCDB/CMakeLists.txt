include(cmake/XRay.CDB.Engine.cmake)
include(cmake/XRay.CDB.Opcode.cmake)

add_library(xrCDB STATIC)

set_property(
  TARGET xrCDB
  PROPERTY FOLDER
  ${FOLDER_XRAY_CDB}
)

target_sources(xrCDB
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

target_include_directories(xrCDB
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(xrCDB
  PRIVATE stdafx.h
)

target_compile_definitions(xrCDB
  PRIVATE
  XRCDB_EXPORTS
)

target_link_libraries(xrCDB
  PRIVATE
  xrAPI
  xrCore
  xrCDB.Engine
  xrCDB.Opcode
  xrRender
)
