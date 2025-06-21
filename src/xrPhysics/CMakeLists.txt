add_library(xrPhysics STATIC)

target_sources(xrPhysics PRIVATE
    ../xrServerEntities/PHNetState.cpp
    ActorCameraCollision.cpp
    collisiondamagereceiver.cpp
    console_vars.cpp
    dcylinder/dCylinder.cpp
    debug_output.cpp
    DisablingParams.cpp
    dRayMotions.cpp
    ElevatorState.cpp
    ExtendedGeom.cpp
    Geometry.cpp
    GeometryBits.cpp
    IActivationShape.cpp
    MathUtils.cpp
    MovementBoxDynamicActivate.cpp
    params.cpp
    PHActivationShape.cpp
    PHActorCharacter.cpp
    PHAICharacter.cpp
    PHCapture.cpp
    PHCaptureInit.cpp
    PHCharacter.cpp
    PHCollideValidator.cpp
    PHContactBodyEffector.cpp
    PHDisabling.cpp
    PHDynamicData.cpp
    PHElement.cpp
    PHElementNetState.cpp
    PHFracture.cpp
    PHGeometryOwner.cpp
    PHInterpolation.cpp
    PHIsland.cpp
    PHJoint.cpp
    PHJointDestroyInfo.cpp
    PHMoveStorage.cpp
    PHObject.cpp
    PHShell.cpp
    PHShellActivate.cpp
    PHShellNetState.cpp
    PHShellSplitter.cpp
    PHSimpleCharacter.cpp
    PHSplitedShell.cpp
    PHStaticGeomShell.cpp
    phvalide.cpp
    PHWorld.cpp
    Physics.cpp
    PhysicsExternalCommon.cpp
    PhysicsShell.cpp
    PhysicsShellAnimator.cpp
    physics_scripted.cpp
    ShellHit.cpp
    stdafx.cpp
    tri-colliderknoopc/dcTriListCollider.cpp
    tri-colliderknoopc/dSortTriPrimitive.cpp
    tri-colliderknoopc/dTriBox.cpp
    tri-colliderknoopc/dTriCallideK.cpp
    tri-colliderknoopc/dTriCylinder.cpp
    tri-colliderknoopc/dTriList.cpp
    tri-colliderknoopc/dTriSphere.cpp
    xrPhysics.cpp
)

target_precompile_headers(xrPhysics
  PRIVATE stdafx.h
)

target_include_directories(xrPhysics
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/sdk/include"
)

target_compile_definitions(xrPhysics
  PUBLIC
  NDEBUG
  dSINGLE
  XRPHYSICS_EXPORTS

  PRIVATE
  WIN32
  MSVC
  _WINDOWS
  _USRDLL
  _SECURE_SCL=0
)

target_precompile_headers(xrPhysics
  PRIVATE stdafx.h
)

enable_parallel_build(xrPhysics)
disable_exceptions()
silence_stdext_hash_deprecation_warnings(xrPhysics)
