add_module(XRay.Engine.Game
  CHILD_OF XRay.Engine

  SOURCES
  IGame_Level.cpp
  IGame_Level_check_textures.cpp
  IGame_Persistent.cpp
  IGame_Level.h
  IGame_Persistent.h

  xrLevel.h
)

add_module(XRay.Engine.Game.Objects
  CHILD_OF XRay.Engine

  SOURCES
  IGame_ObjectPool.cpp
  IGame_ObjectPool.h

  pure_relcase.cpp
  pure_relcase.h

  xr_object.cpp
  xr_object.h
  
  xr_object_list.cpp
  xr_object_list.h

  ShadersExternalData.h
)

add_module(XRay.Engine.Game.HUD
  CHILD_OF XRay.Engine

  SOURCES
  CustomHUD.cpp
  CustomHUD.h
)

add_module(XRay.Engine.Game.Materials
  CHILD_OF XRay.Engine
  
  SOURCES
  GameMtlLib.cpp
  GameMtlLib_Engine.cpp
  GameMtlLib.h
)

add_module(XRay.Engine.Game.Feelers
  CHILD_OF XRay.Engine
  
  SOURCES
  Feel_Sound.h

  Feel_Touch.cpp
  Feel_Touch.h

  Feel_Vision.cpp
  Feel_Vision.h
)

add_module(XRay.Engine.Game.Environment
  CHILD_OF XRay.Engine
  
  SOURCES
  Environment_misc.cpp
  Environment_render.cpp
  Environment.cpp
  Environment.h

  xrHemisphere.cpp
  xrHemisphere.h
)

add_module(XRay.Engine.Game.Environment.Effects
  CHILD_OF XRay.Engine
  
  SOURCES
  Rain.cpp
  Rain.h

  thunderbolt.cpp
  thunderbolt.h

  xr_efflensflare.cpp
  xr_efflensflare.h
)

add_module(XRay.Engine.Game.Demo
  CHILD_OF XRay.Engine
  
  SOURCES
  FDemoPlay.cpp
  FDemoRecord.cpp
  FDemoPlay.h
  FDemoRecord.h
)

add_module(XRay.Engine.Game.Debug
  CHILD_OF XRay.Engine
  
  SOURCES
  ObjectDump.cpp
  ObjectDump.h
)

add_module(XRay.Engine.Game.Collision
  CHILD_OF XRay.Engine

  SOURCES
  cf_dynamic_mesh.cpp
  cf_dynamic_mesh.h

  xr_collide_form.cpp
  xr_collide_form.h
)

add_module(XRay.Engine.Game.Cameras
  CHILD_OF XRay.Engine

  SOURCES
  CameraBase.cpp
  CameraManager.cpp
  CameraBase.h
  CameraDefs.h
  CameraManager.h

  Effector.cpp
  EffectorPP.cpp
  Effector.h
  EffectorPP.h
)

add_module(XRay.Engine.Game.Animator
  CHILD_OF XRay.Engine

  SOURCES
  bone.cpp
  bone.h

  Envelope.cpp
  envelope.h

  interp.cpp

  motion.cpp
  motion.h

  ObjectAnimator.cpp
  ObjectAnimator.h
)
