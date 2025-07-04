add_module(XRay.Engine.Interfaces.Collidable
  CHILD_OF XRay.Engine

  SOURCES
  ICollidable.cpp
  ICollidable.h
)

add_module(XRay.Engine.Interfaces.Particles
  CHILD_OF XRay.Engine

  SOURCES
  psystem.h
)

add_module(XRay.Engine.Interfaces.ImGui
  CHILD_OF XRay.Engine

  SOURCES
  imgui_base.cpp
  imgui_base_input.cpp
  imgui_helper.cpp

  imgui_base.h
  imgui_helper.h
)

add_module(XRay.Engine.Interfaces.Input
  CHILD_OF XRay.Engine

  SOURCES
  IInputReceiver.cpp
  IInputReceiver.h
  
  Xr_input.cpp
  xr_input_xinput.cpp
  xr_input.h
  xr_input_xinput.h
)

add_module(XRay.Engine.Interfaces.Physics
  CHILD_OF XRay.Engine

  SOURCES
  phdebug.cpp
  
  IObjectPhysicsCollision.h
  IPHdebug.h
  IPhysicsGeometry.h
  IPhysicsShell.h
)

add_module(XRay.Engine.Interfaces.Render
  CHILD_OF XRay.Engine

  SOURCES
  IRenderable.cpp
  Render.cpp
  IRenderable.h
  Render.h
)

add_module(XRay.Engine.Interfaces.Scheduler
  CHILD_OF XRay.Engine

  SOURCES
  ISheduled.cpp
  xrSheduler.cpp
  ISheduled.h
  xrSheduler.h
)
