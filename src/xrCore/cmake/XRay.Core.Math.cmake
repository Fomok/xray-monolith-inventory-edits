add_library(XRay.Core.Math STATIC)

set_property(
  TARGET XRay.Core.Math
  PROPERTY FOLDER
  ${FOLDER_XRAY_CORE}
)

target_sources(XRay.Core.Math
  PRIVATE
  cpuid.cpp
  
  cpuid.h
  
  buffer_vector.h
  buffer_vector_inline.h
  
  _std_extensions.cpp
  clsid.cpp
  string_concatenations.cpp
  xr_trims.cpp
  
  _std_extensions.h
  _stl_extensions.h
  _type_traits.h
  clsid.h
  fastdelegate.h
  string_concatenations.h
  string_concatenations_inline.h
  xr_trims.h
  
  FixedMap.h
  FixedSet.h
  FixedVector.h
  xrPool.h
  dump_string.h
  
  _compressed_normal.cpp
  _math.cpp
  _sphere.cpp
  dump_string.cpp
  
  _bitwise.h
  _color.h
  _compressed_normal.h
  _cylinder.h
  _fbox.h
  _fbox2.h
  _flags.h
  _math.h
  _matrix.h
  _matrix33.h
  _obb.h
  _plane.h
  _plane2.h
  _quaternion.h
  _random.h
  _rect.h
  _sphere.h
  _types.h
  _vector2.h
  _vector3d.h
  _vector3d_ext.h
  _vector4.h
  vector.h
)

target_include_directories(XRay.Core.Math
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core.Math
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core.Math
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core.Math
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.CDB
)
