add_module(XRay.Game.Core
  TYPE STATIC

  LINKS
  IKAN

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
