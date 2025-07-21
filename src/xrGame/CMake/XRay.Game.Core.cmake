add_module(XRay.Game.Core
  TYPE STATIC

  DEFINES
  XRCORE_EXPORTS

  DEPENDS
  XRay.Collision
  XRay.Core.Crypto
  XRay.Engine
  XRay.NetServer
  XRay.Render.API
  XRay.Render.Common

  LINKS
  IKAN
  ode
  luabind
  LuaJIT
  LZO
  TinyXML

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
