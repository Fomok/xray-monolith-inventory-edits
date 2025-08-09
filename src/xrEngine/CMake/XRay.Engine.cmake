add_module(XRay.Engine
  TYPE STATIC
  
  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}

  DEFINES
  ENGINE_BUILD
  [[ENGINE_API=]]
  [[ECORE_API=]]
  [[DLL_API=]]
  $<$<CONFIG:DEBUG>:D3D_DEBUG_INFO>
  $<$<NOT:$<CONFIG:DEBUG>>:LUABIND_NO_ERROR_CHECKING>

  LINKS
  dxsdk
  discord
  dinput8
  imgui
  libogg
  libtheora
  LuaJIT
  luabind
  Vfw32
  
  XRay.Core.Defines

  XRay.Collision.Includes
  XRay.Core.Includes
  XRay.Game.Includes
  XRay.Particles.Includes
  XRay.Render.Common.Includes
  XRay.Render.API.Includes
  XRay.Sound.Includes
  
  XRay.Game.AI.Navigation

  PRECOMPILES
  #[["xrAPI.h"]]
  #[["d3d9.h"]]
  #[["bone.h"]]
  #[["Render.h"]]
  #[["Device.h"]]
  #[["light.h"]]
  #[["defines.h"]]
  #[["fs.h"]]
  #[["xrXRC.h"]]
  #[["sound.h"]]
  #[["sound.h"]]

  SOURCES
  defines.cpp
  pure.cpp

  dedicated_server_only.h
  defines.h
  mp_logging.h
  no_single.h
  Properties.h
  pure.h
  std_classes.h
  trivial_encryptor.h
  _d3d_extensions.h
)


# you must define ENGINE_BUILD then building the engine itself
# and not define it if you are about to build DLL

#ifndef NO_ENGINE_API
#ifdef ENGINE_BUILD
#define DLL_API
#__declspec(dllimport)
#define ENGINE_API
#__declspec(dllexport)
#else
#undef DLL_API
#define DLL_API
#__declspec(dllexport)
#define ENGINE_API
#__declspec(dllimport)
#endif
#else
#define ENGINE_API
#define DLL_API
#endif // NO_ENGINE_API

if(NOT NO_XRLOG)
  #target_precompile_headers(XRay.Engine.Precompiles
  #  INTERFACE
  #  [["log.h"]]
  #)
endif()

if($<OR:$<NOT:$<CONFIG:DEBUG>>,${FORCE_NO_EXCEPTIONS}>)
  target_compile_definitions(XRay.Engine.Defines
    INTERFACE
    BOOST_NO_EXCEPTIONS
  )
endif()

add_module(XRay.Engine.Collision
  SOURCES
  cl_intersect.h
)

add_module(XRay.Engine.Console
  SOURCES
  XR_IOConsole.cpp
  XR_IOConsole_callback.cpp
  XR_IOConsole_control.cpp
  XR_IOConsole_get.cpp
  XR_IOConsole.h
  
  xr_ioc_cmd.cpp
  xr_ioc_cmd.h
)

add_module(XRay.Engine.Core
  SOURCES
  Engine.cpp
  Engine.h

  EventAPI.cpp
  EventAPI.h

  mailSlot.cpp
)

add_module(XRay.Engine.OpenAutomate
  SOURCES
  xrSASH.cpp
  xrSASH.h
)

add_module(XRay.Engine.Noise
  SOURCES
  perlin.cpp
  perlin.h
)

add_module(XRay.Engine.Scripting
  SOURCES
  _scripting.cpp
  ai_script_lua_debug.cpp
  ai_script_lua_extension.cpp
  ai_script_lua_extension.h
  ai_script_lua_space.h
  ai_script_space.h
)

add_module(XRay.Engine.TextEditor
  SOURCES
  edit_actions.cpp
  edit_actions.h

  line_editor.cpp
  line_editor.h
  
  line_edit_control.cpp
  line_edit_control.h
)

set_source_files_properties(
  xrTheora_Surface.cpp
  PROPERTIES
  SKIP_UNITY_BUILD_INCLUSION true
)

include(XRay.Engine.Interfaces)
include(XRay.Engine.Render)
include(XRay.Engine.Game)
include(XRay.Engine.Main)
include(XRay.Engine.Targets)
