add_library(XRay.Game.Physics STATIC)

set_property(
  TARGET XRay.Game.Physics
  PROPERTY FOLDER
  ${FOLDER_XRAY_GAME}
)

target_sources(XRay.Game.Physics PRIVATE
  ../xrServerEntities/PHNetState.cpp
  PHCollisionDamageReceiver.cpp
  PHCommander.cpp
  PHDebug.cpp
  PHDestroyable.cpp
  PHDestroyableNotificate.cpp
  PHMovementControl.cpp
  PHMovementDynamicActivate.cpp
  PHScriptCall.cpp
  PHShellCreator.cpp
  PHSimpleCalls.cpp
  PHSimpleCallsScript.cpp
  PHSkeleton.cpp
  PHSoundPlayer.cpp
  PhysicObject.cpp
  PhysicObject_script.cpp
  PhysicsGamePars.cpp
  PhysicsShellHolder.cpp
  PhysicsSkeletonObject.cpp
  physics_element_scripted.cpp
  physics_game.cpp
  physics_joint_scripted.cpp
  physics_shell_animated.cpp
  physics_shell_scripted.cpp
  physics_world_scripted.cpp
  physic_item.cpp
  
  ../xrServerEntities/PHNetState.h
  ../xrServerEntities/PHSynchronize.h
  PHCollisionDamageReceiver.h
  PHCommander.h
  PHDebug.h
  PHDestroyable.h
  PHDestroyableNotificate.h
  PHMovementControl.h
  PHReqComparer.h
  PHScriptCall.h
  PHShellCreator.h
  PHSimpleCalls.h
  PHSkeleton.h
  PHSoundPlayer.h
)

target_include_directories(XRay.Game.Physics
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE
  ${CMAKE_SOURCE_DIR}/src/xrServerEntities
)

target_compile_definitions(XRay.Game.Physics
  PRIVATE
  XRGAME_EXPORTS
)

target_precompile_headers(XRay.Game.Physics
  PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:stdafx.h>
)

target_link_libraries(XRay.Game.Physics
  PRIVATE
  DPlay
  imgui
  luabind
  LuaJIT
  XRay.Collision
  XRay.Core
  XRay.Engine
  XRay.Render
  XRay.Render.API
  XRay.Physics
  XRay.Sound
  XRay.NetServer
)
