add_module(XRay.Game
  TYPE STATIC
  
  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/xrServerEntities

  PRECOMPILES
  ../xrEngine/stdafx.h

  [["DPlay/dplay8.h"]]

  ../build_config_defines.h

  macros.h

  [["gamefont.h"]]
  [["xr_object.h"]]
  [["igame_level.h"]]
  [["xrPhysics.h"]]
  [["smart_cast.h"]]
  
  DEFINES
  GP_EMAIL_LEN=128
  GP_NICK_LEN=32
  GP_UNIQUENICK_LEN=32
  GP_PASSWORD_LEN=32
  XRGAME_EXPORTS

  LINKS
  CxImage
  IKAN
  XRay.Collision
  XRay.Core
  XRay.Core.Crypto
  XRay.Engine
  XRay.NetServer
  XRay.Physics

  SOURCES
  xrGame.cpp
)

include(XRay.Game.AI)
include(XRay.Game.Core)
include(XRay.Game.Physics)
include(XRay.Game.UI)