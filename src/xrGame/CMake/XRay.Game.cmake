add_module(XRay.Game
  TYPE STATIC
  
  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/xrServerEntities

  PRECOMPILES $<$<COMPILE_LANGUAGE:CXX>:stdafx.h>
  
  DEFINES XRGAME_EXPORTS

  LINKS
  crypto
  CxImage
  imgui
  loki
  lua-extensions
  luabind
  LuaJIT
  ode
  optick
  XRay.Collision
  XRay.Core
  XRay.Engine
  XRay.NetServer
  XRay.Physics
  XRay.Render.Common
  XRay.Render.API

  SOURCES
  xrGame.cpp
)

include(XRay.Game.AI)
include(XRay.Game.Core)
include(XRay.Game.Physics)
include(XRay.Game.UI)