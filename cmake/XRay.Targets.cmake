include_guard()

set(XR_ENGINE_SOURCES
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

  ai_script_lua_extension.h
  ai_script_lua_space.h
  ai_script_space.h
  bone.h
  CameraBase.h
  CameraDefs.h
  CameraManager.h
  cf_dynamic_mesh.h
  cl_intersect.h
  CustomHUD.h
  dedicated_server_only.h
  defines.h
  device.h
  edit_actions.h
  Effector.h
  EffectorPP.h
  Engine.h
  EngineAPI.h
  EnnumerateVertices.h
  envelope.h
  Environment.h
  EventAPI.h
  FDemoPlay.h
  FDemoRecord.h
  Feel_Sound.h
  Feel_Touch.h
  Feel_Vision.h
  Fmesh.h
  GameFont.h
  GameMtlLib.h
  ICollidable.h
  IGame_Level.h
  IGame_ObjectPool.h
  IGame_Persistent.h
  IInputReceiver.h
  imgui_base.h
  imgui_helper.h
  IObjectPhysicsCollision.h
  IPHdebug.h
  IPhysicsGeometry.h
  IPhysicsShell.h
  IRenderable.h
  ISheduled.h
  LightAnimLibrary.h
  line_editor.h
  line_edit_control.h
  MbHelpers.h
  motion.h
  mp_logging.h
  no_single.h
  ObjectAnimator.h
  ObjectDump.h
  perlin.h
  Properties.h
  psystem.h
  PS_instance.h
  pure.h
  pure_relcase.h
  Rain.h
  Render.h
  resource.h
  ShadersExternalData.h
  Shader_xrLC.h
  SkeletonMotionDefs.h
  SkeletonMotions.h
  StatGraph.h
  Stats.h
  stdafx.h
  std_classes.h
  Text_Console.h
  thunderbolt.h
  tntQAVI.h
  trivial_encryptor.h
  vis_common.h
  WaveForm.h
  xrHemisphere.h
  xrImage_Resampler.h
  xrLevel.h
  xrSASH.h
  xrSheduler.h
  xrTheora_Stream.h
  xrTheora_Surface.h
  xrTheora_Surface_mmx.h
  xr_collide_form.h
  xr_efflensflare.h
  xr_input.h
  xr_input_xinput.h
  XR_IOConsole.h
  xr_ioc_cmd.h
  xr_object.h
  xr_object_list.h
  x_ray.h
  _d3d_extensions.h

  resource.rc
  dpi-aware.manifest
)

# Given a list of sources, prepend the xrEngine source dir to each,
# and add it to the given target
function(target_engine_sources TARGET)
  foreach(src ${ARGN})
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/xrEngine/${src}")
  endforeach()
  target_sources(${TARGET} PRIVATE ${SOURCES})
endfunction()

# Given a list of precompiled headers, prepend the xrEngine source dir to each,
# and add it to the given target
function(target_engine_precompile_headers TARGET)
  foreach(src ${ARGN})
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/xrEngine/${src}")
  endforeach()
  target_precompile_headers(${TARGET} PRIVATE ${SOURCES})
endfunction()

# Add an xrEngine target with the given name
macro(add_engine_target name)
  add_executable(${name} WIN32)

  set_property(
    TARGET ${name}
    PROPERTY FOLDER
    ${FOLDER_TARGETS}
  )

  target_engine_sources(${name} ${XR_ENGINE_SOURCES})
  target_engine_precompile_headers(${name} stdafx.h)

  target_include_directories(${name}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/xrEngine
  )

  target_compile_definitions(${name}
    PUBLIC
    ENGINE_BUILD
  )

  target_link_libraries(${name}
    PUBLIC
    discord
    dinput8
    icu
    imgui
    lua-extensions
    ReShadeCompat
    Vfw32
    xrCore
    xrGame
    xrParticles
    xrSound
    xrXMLParser
  )

  set(PDB_ZIP ${name}_pdb.zip)
  add_custom_target(${name}-PDB)

  set_property(
    TARGET ${name}-PDB
    PROPERTY FOLDER
    ${FOLDER_RELEASE}
  )

  target_sources(${name}-PDB
    PRIVATE
    ${PDB_ZIP}
  )

  add_custom_command(
      OUTPUT ${PDB_ZIP}
      COMMAND ${CMAKE_COMMAND} -E tar "cf" "${PDB_ZIP}" --format=zip "$<TARGET_PDB_FILE:${name}>"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Zip $<TARGET_PDB_FILE:${name}> to ${CMAKE_BINARY_DIR}/${PDB_ZIP}."
      VERBATIM
  )

  add_dependencies(${name}-PDB ${name})
endmacro()

macro(configure_engine_target TARGET DEFINE LINK)
  target_compile_definitions(${TARGET}
    PRIVATE
    ${DEFINE}
  )
  target_link_libraries(${TARGET} PRIVATE ${LINK})
endmacro()

# Enable AVX for the given target
macro(enable_avx TARGET)
  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /arch:AVX)
  endif()
endmacro()

# Setup executable targets
add_engine_target(AnomalyDX8)
configure_engine_target(AnomalyDX8 STATIC_RENDERER_R1 xrRender_R1)

add_engine_target(AnomalyDX8AVX)
enable_avx(AnomalyDX8AVX)
configure_engine_target(AnomalyDX8AVX STATIC_RENDERER_R1 xrRender_R1)

add_engine_target(AnomalyDX9)
configure_engine_target(AnomalyDX9 STATIC_RENDERER_R2 xrRender_R2)

add_engine_target(AnomalyDX9AVX)
enable_avx(AnomalyDX9AVX)
configure_engine_target(AnomalyDX9AVX STATIC_RENDERER_R2 xrRender_R2)

add_engine_target(AnomalyDX10)
configure_engine_target(AnomalyDX10 STATIC_RENDERER_R3 xrRender_R3)

add_engine_target(AnomalyDX10AVX)
enable_avx(AnomalyDX10AVX)
configure_engine_target(AnomalyDX10AVX STATIC_RENDERER_R3 xrRender_R3)

add_engine_target(AnomalyDX11)
configure_engine_target(AnomalyDX11 STATIC_RENDERER_R4 xrRender_R4)

add_engine_target(AnomalyDX11AVX)
enable_avx(AnomalyDX11AVX)
configure_engine_target(AnomalyDX11AVX STATIC_RENDERER_R4 xrRender_R4)
