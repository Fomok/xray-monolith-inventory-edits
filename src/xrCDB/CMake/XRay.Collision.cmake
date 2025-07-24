add_module(XRay.Collision
  TYPE STATIC

  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}

  DEFINES
  XRCDB_EXPORTS

  LINKS
  OPCODE
  optick

  XRay.Core.Defines
  
  XRay.Core.Includes
  XRay.Engine.Includes
  XRay.Render.API.Includes
  XRay.Render.Common.Includes

  PRECOMPILES
  stdafx.h
  
  SOURCES
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

target_compile_options(XRay.Collision
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/wd4458>
)

set_source_files_properties(
  ISpatial_q_frustum.cpp
  ISpatial_q_ray.cpp
  ISpatial_verify.cpp
  OPC_OBBCollider.cpp
  OPC_TreeCollider.cpp
  PROPERTIES
  SKIP_UNITY_BUILD_INCLUSION true
)

include(XRay.Collision.Engine)
