add_executable(xrEngine)

target_sources(xrEngine PRIVATE
    "../3rd party/discord/achievement_manager.cpp"
    "../3rd party/discord/activity_manager.cpp"
    "../3rd party/discord/application_manager.cpp"
    "../3rd party/discord/core.cpp"
    "../3rd party/discord/image_manager.cpp"
    "../3rd party/discord/lobby_manager.cpp"
    "../3rd party/discord/network_manager.cpp"
    "../3rd party/discord/overlay_manager.cpp"
    "../3rd party/discord/relationship_manager.cpp"
    "../3rd party/discord/storage_manager.cpp"
    "../3rd party/discord/store_manager.cpp"
    "../3rd party/discord/types.cpp"
    "../3rd party/discord/user_manager.cpp"
    "../3rd party/discord/voice_manager.cpp"
    ai_script_lua_debug.cpp
    ai_script_lua_extension.cpp
    bone.cpp
    CameraBase.cpp
    CameraManager.cpp
    cf_dynamic_mesh.cpp
    CustomHUD.cpp
    defines.cpp
    device.cpp
    Device_create.cpp
    Device_destroy.cpp
    Device_Initialize.cpp
    Device_Misc.cpp
    Device_overdraw.cpp
    Device_wndproc.cpp
    edit_actions.cpp
    Effector.cpp
    EffectorPP.cpp
    Engine.cpp
    EngineAPI.cpp
    Envelope.cpp
    Environment.cpp
    Environment_misc.cpp
    Environment_render.cpp
    EventAPI.cpp
    FDemoPlay.cpp
    FDemoRecord.cpp
    Feel_Touch.cpp
    Feel_Vision.cpp
    fmesh.cpp
    GameFont.cpp
    GameMtlLib.cpp
    GameMtlLib_Engine.cpp
    ICollidable.cpp
    IGame_Level.cpp
    IGame_Level_check_textures.cpp
    IGame_ObjectPool.cpp
    IGame_Persistent.cpp
    IInputReceiver.cpp
    imgui_base.cpp
    imgui_base_input.cpp
    imgui_helper.cpp
    interp.cpp
    IRenderable.cpp
    ISheduled.cpp
    LightAnimLibrary.cpp
    line_editor.cpp
    line_edit_control.cpp
    mailSlot.cpp
    MbHelpers.cpp
    motion.cpp
    ObjectAnimator.cpp
    ObjectDump.cpp
    perlin.cpp
    phdebug.cpp
    PS_instance.cpp
    pure.cpp
    pure_relcase.cpp
    Rain.cpp
    Render.cpp
    SkeletonMotions.cpp
    StatGraph.cpp
    Stats.cpp
    stdafx.cpp
    Text_Console.cpp
    Text_Console_WndProc.cpp
    thunderbolt.cpp
    tntQAVI.cpp
    xrHemisphere.cpp
    xrImage_Resampler.cpp
    xrSASH.cpp
    xrSheduler.cpp
    xrTheora_Stream.cpp
    xrTheora_Surface.cpp
    xrTheora_Surface_mmx.cpp
    xr_collide_form.cpp
    xr_efflensflare.cpp
    Xr_input.cpp
    xr_input_xinput.cpp
    XR_IOConsole.cpp
    XR_IOConsole_callback.cpp
    XR_IOConsole_control.cpp
    XR_IOConsole_get.cpp
    xr_ioc_cmd.cpp
    xr_object.cpp
    xr_object_list.cpp
    x_ray.cpp
    _scripting.cpp
)

target_precompile_headers(xrEngine
  PRIVATE stdafx.h
)

target_include_directories(xrEngine PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/src/3rd party"
  "${CMAKE_SOURCE_DIR}/src/3rd party/luajit-2/src"
  "${CMAKE_SOURCE_DIR}/src/3rd party/luabind"
  "${CMAKE_SOURCE_DIR}/src/3rd party/imgui"
  "${CMAKE_SOURCE_DIR}/src/3rd party/icu/include"
  "${CMAKE_SOURCE_DIR}/src/xrServerEntities"
  "${CMAKE_SOURCE_DIR}/sdk/include"
  "${CMAKE_SOURCE_DIR}/sdk/include/dxsdk"
)

target_compile_definitions(xrEngine
  PUBLIC
  NDEBUG
  ENGINE_BUILD
  STATIC_RENDERER_R4

  PRIVATE
  WIN32
  _WINDOWS
  _SECURE_SCL=0
)

target_link_directories(xrEngine
  PRIVATE
  "${CMAKE_SOURCE_DIR}/sdk/libraries/x64"
  "${CMAKE_SOURCE_DIR}/src/3rd party/stackwalker/lib"
)

target_link_libraries(xrEngine
  xrCore
  xrCDB
  xrSound
  luajit-2
  luabind
  xrAPI
  xrRender_R4
  xrNetServer
  ReShadeCompat
  imgui
  xrParticles
  xrCPU_Pipe
  lua-extensions
  xrGame
  xrPhysics
  xrXMLParser
  crypto
  CxImage
  libjpeg
  ode
  libogg_static
  libtheora_static
  libvorbis_static
  libvorbisfile_static
  OpenAL32

  StackWalker
)

enable_parallel_build(xrEngine)
disable_exceptions(xrEngine)
silence_stdext_hash_deprecation_warnings(xrEngine)
