set(ODE_DOUBLE_PRECISION Off CACHE BOOL "Use double-precision floating point for collision logic.")

add_library(ode STATIC)

set_property(
  TARGET ode
  PROPERTY FOLDER
  ${FOLDER_3RD_PARTY}
)

target_sources(ode PRIVATE
  ode/src/array.cpp
  ode/src/Bounder33.cpp
  ode/src/collision_kernel.cpp
  ode/src/collision_quadtreespace.cpp
  ode/src/collision_space.cpp
  ode/src/collision_std.cpp
  ode/src/collision_transform.cpp
  ode/src/collision_util.cpp
  ode/src/error.cpp
  ode/src/export-dif.cpp
  ode/src/fastdot.c
  ode/src/fastldlt.c
  ode/src/fastlsolve.c
  ode/src/fastltsolve.c
  ode/src/joint.cpp
  ode/src/lcp.cpp
  ode/src/mass.cpp
  ode/src/mat.cpp
  ode/src/matrix.cpp
  ode/src/memory.cpp
  ode/src/misc.cpp
  ode/src/obstack.cpp
  ode/src/ode.cpp
  ode/src/odemath.cpp
  ode/src/quickstep.cpp
  ode/src/rotation.cpp
  ode/src/step.cpp
  ode/src/stepfast.cpp
  ode/src/StepJointInternal.cpp
  ode/src/testing.cpp
  ode/src/timer.cpp
  ode/src/util.cpp
  contrib/msvc7/ode_default/de_padf_integration.cpp

  include/ode/collision.h
  include/ode/collision_space.h
  include/ode/collision_trimesh.h
  include/ode/common.h
  include/ode/compatibility.h
  include/ode/config.h
  include/ode/contact.h
  include/ode/error.h
  include/ode/export-dif.h
  include/ode/geom.h
  include/ode/mass.h
  include/ode/matrix.h
  include/ode/memory.h
  include/ode/misc.h
  include/ode/objects.h
  include/ode/ode.h
  include/ode/odecpp.h
  include/ode/odecpp_collision.h
  include/ode/odemath.h
  include/ode/rotation.h
  include/ode/space.h
  include/ode/timer.h
  ode/src/array.h
  ode/src/Bounder33.h
  ode/src/collision_kernel.h
  ode/src/collision_space_internal.h
  ode/src/collision_std.h
  ode/src/collision_transform.h
  ode/src/collision_trimesh_internal.h
  ode/src/collision_util.h
  ode/src/geom_internal.h
  ode/src/joint.h
  ode/src/lcp.h
  ode/src/Lcp33.h
  ode/src/mat.h
  ode/src/objects.h
  ode/src/obstack.h
  ode/src/quickstep.h
  ode/src/stack.h
  ode/src/step.h
  ode/src/StepJointInternal.h
  ode/src/testing.h
  ode/src/util.h
  contrib/msvc7/ode_default/de_padf_integration.h
)

target_include_directories(ode
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(ODE_DOUBLE_PRECISION)
  target_compile_definitions(ode PUBLIC dDOUBLE)
else()
  target_compile_definitions(ode PUBLIC dSINGLE)
endif()


enable_parallel_build(ode)
