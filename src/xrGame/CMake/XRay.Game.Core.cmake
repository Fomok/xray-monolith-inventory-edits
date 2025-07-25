add_module(XRay.Game.Core
  TYPE STATIC

  LINKS
  CxImage
  IKAN
  luabind
  LuaJIT
  LZO
  ode
  TinyXML

  XRay.Core.Defines
  XRay.Game.Defines

  XRay.Includes
  XRay.Collision.Includes
  XRay.Core.Includes
  XRay.Core.Crypto.Includes
  XRay.CPUPipe.Includes
  XRay.Engine.Includes
  XRay.Game.Includes
  XRay.NetServer.Includes
  XRay.Physics.Includes
  XRay.Render.API.Includes
  XRay.Render.Common.Includes
  XRay.ServerEntities.Includes

  XRay.Game.Precompiles

  SOURCES
  ../xrServerEntities/pch_script.cpp
  ../xrServerEntities/pch_script.h
)

target_compile_options(XRay.Game.Core
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
)

include(XRay.Game.Core.Client)
include(XRay.Game.Core.Common)
include(XRay.Game.Core.Server)
