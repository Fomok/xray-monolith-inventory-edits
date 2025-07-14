add_module(XRay.Game.Core
  TYPE STATIC
  ../xrServerEntities/pch_script.cpp
  ../xrServerEntities/pch_script.h
)

include(XRay.Game.Core.Client)
include(XRay.Game.Core.Common)
include(XRay.Game.Core.Server)
