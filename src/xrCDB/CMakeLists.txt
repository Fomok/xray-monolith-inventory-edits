add_library(xrCDB STATIC)

target_sources(xrCDB PRIVATE
    Frustum.cpp
    ISpatial.cpp
    ISpatial_q_box.cpp
    ISpatial_q_frustum.cpp
    ISpatial_q_ray.cpp
    ISpatial_verify.cpp
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
    StdAfx.cpp
    xrCDB.cpp
    xrCDB_box.cpp
    xrCDB_Collector.cpp
    xrCDB_frustum.cpp
    xrCDB_ray.cpp
    xrXRC.cpp
    xr_area.cpp
    xr_area_query.cpp
    xr_area_raypick.cpp
)

target_include_directories(xrCDB
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/sdk/include"
  "${CMAKE_SOURCE_DIR}/src/3rd party"
)

target_precompile_headers(ReShadeCompat
  PRIVATE stdafx.h
)

target_compile_definitions(xrCDB
  PUBLIC
  NDEBUG
  XRCDB_EXPORTS

  PRIVATE
  WIN32
  _WINDOWS
  _USRDLL
  _SECURE_SCL=0
)

enable_parallel_build(xrCDB)
disable_exceptions()
silence_stdext_hash_deprecation_warnings(xrCDB)
