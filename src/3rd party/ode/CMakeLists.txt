add_library(ode STATIC)

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
)

target_include_directories(ode
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/ode/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_definitions(ode
  PUBLIC
  NDEBUG
  dNODEBUG
  dSINGLE

  PRIVATE
  WIN32
  MSVC
  _WINDOWS
  _SECURE_SCL=0
)

enable_parallel_build(ode)
