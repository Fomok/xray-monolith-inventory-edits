add_module(XRay.Collision
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

  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}

  PRECOMPILES stdafx.h

  DEFINES XRCDB_EXPORTS

  LINKS
  XRay.Core
  XRay.Render
  XRay.Render.API
)

include(XRay.Collision.Engine)
include(XRay.Collision.Opcode)
