# X-Ray entrypoint
# Link this to a Win32 executable alongside an XRay.Render.R* module to produce a runnable game
add_library(XRay.Engine.Main INTERFACE)

target_link_libraries(XRay.Engine.Main
  INTERFACE
  discord
  icu

  XRay.Core
  XRay.Collision
  XRay.CPUPipe
  XRay.Engine
  XRay.Game
  XRay.NetServer
  XRay.Particles
  XRay.Physics
  XRay.Render.API
  XRay.Sound
  XRay.XMLParser
)

target_sources(XRay.Engine.Main
  INTERFACE
  x_ray.cpp
  x_ray.h
  
  EngineAPI.cpp
  EngineAPI.h

  resource.h
  resource.rc
  
  dpi-aware.manifest
)
