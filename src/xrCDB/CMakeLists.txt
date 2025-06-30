add_library(xrCDB STATIC)

set_property(
  TARGET xrCDB
  PROPERTY FOLDER
  ${FOLDER_XRAY}
)

target_sources(xrCDB
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

  ISpatial.h
  xr_area.h
  xr_collide_defs.h
  xrXRC.h

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

  Opcode.cpp
  OPC_AABB.cpp
  OPC_AABBCollider.cpp
  OPC_AABBTree.cpp
  OPC_Collider.cpp
  OPC_Common.cpp
  OPC_Container.cpp
  OPC_Matrix3x3.cpp
  OPC_Matrix4x4.cpp
  OPC_Model.cpp
  OPC_OBB.cpp
  OPC_OBBCollider.cpp
  OPC_OptimizedTree.cpp
  OPC_Plane.cpp
  OPC_PlanesCollider.cpp
  OPC_Point.cpp
  OPC_Ray.cpp
  OPC_RayCollider.cpp
  OPC_SphereCollider.cpp
  OPC_TreeBuilders.cpp
  OPC_TreeCollider.cpp
  OPC_Triangle.cpp
  OPC_VolumeCollider.cpp
  
  Opcode.h
  OPC_AABB.h
  OPC_AABBCollider.h
  OPC_AABBTree.h
  OPC_BoundingSphere.h
  OPC_BoxBoxOverlap.h
  OPC_BVTCache.h
  OPC_Collider.h
  OPC_Common.h
  OPC_Container.h
  OPC_FPU.h
  OPC_Matrix3x3.h
  OPC_Matrix4x4.h
  OPC_MemoryMacros.h
  OPC_Model.h
  OPC_OBB.h
  OPC_OBBCollider.h
  OPC_OptimizedTree.h
  OPC_Plane.h
  OPC_PlanesAABBOverlap.h
  OPC_PlanesCollider.h
  OPC_PlanesTriOverlap.h
  OPC_Point.h
  OPC_Preprocessor.h
  OPC_Ray.h
  OPC_RayAABBOverlap.h
  OPC_RayCollider.h
  OPC_RayTriOverlap.h
  OPC_Settings.h
  OPC_SphereAABBOverlap.h
  OPC_SphereCollider.h
  OPC_SphereTriOverlap.h
  OPC_TreeBuilders.h
  OPC_TreeCollider.h
  OPC_Triangle.h
  OPC_TriBoxOverlap.h
  OPC_TriTriOverlap.h
  OPC_Types.h
  OPC_VolumeCollider.h
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
  xrRender
)
