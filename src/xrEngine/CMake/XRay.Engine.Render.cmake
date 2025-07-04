add_module(XRay.Engine.Render.TextConsole
  CHILD_OF XRay.Engine

  SOURCES
  Text_Console.cpp
  Text_Console_WndProc.cpp
  Text_Console.h
)

add_module(XRay.Engine.Render.Textures
  CHILD_OF XRay.Engine

  SOURCES
  tntQAVI.cpp
  tntQAVI.h

  xrImage_Resampler.cpp
  xrImage_Resampler.h

  xrTheora_Stream.cpp
  xrTheora_Surface.cpp
  xrTheora_Surface_mmx.cpp

  xrTheora_Stream.h
  xrTheora_Surface.h
  xrTheora_Surface_mmx.h
)

add_module(XRay.Engine.Render.Shaders
  CHILD_OF XRay.Engine

  SOURCES
  Shader_xrLC.h
  WaveForm.h
)

add_module(XRay.Engine.Render.Fonts
  CHILD_OF XRay.Engine

  SOURCES
  GameFont.cpp
  GameFont.h
  
  MbHelpers.cpp
  MbHelpers.h
)

add_module(XRay.Engine.Render.Lighting
  CHILD_OF XRay.Engine

  SOURCES
  LightAnimLibrary.cpp
  LightAnimLibrary.h
)

add_module(XRay.Engine.Render.Particles
  CHILD_OF XRay.Engine

  SOURCES
  PS_instance.cpp
  PS_instance.h
)

add_module(XRay.Engine.Render.Visibility
  CHILD_OF XRay.Engine

  SOURCES
  vis_common.h
)

add_module(XRay.Engine.Render.Visuals
  CHILD_OF XRay.Engine

  SOURCES
  fmesh.cpp
  Fmesh.h
)

add_module(XRay.Engine.Render.Visuals.Skeleton
  CHILD_OF XRay.Engine

  SOURCES
  SkeletonMotions.cpp
  SkeletonMotionDefs.h
  SkeletonMotions.h
  EnnumerateVertices.h
)

add_module(XRay.Engine.Render.Device
  CHILD_OF XRay.Engine

  SOURCES
  device.cpp
  Device_create.cpp
  Device_destroy.cpp
  Device_Initialize.cpp
  Device_Misc.cpp
  Device_overdraw.cpp
  Device_wndproc.cpp
  device.h

  StatGraph.cpp
  Stats.cpp
  StatGraph.h
  Stats.h
)
