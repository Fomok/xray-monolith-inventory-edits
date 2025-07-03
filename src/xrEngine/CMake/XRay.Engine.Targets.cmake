include_guard()

# Add an XRay.Engine target with the given name
macro(add_engine_target name)
  add_executable(${name} WIN32)

  target_folder(${name} ${FOLDER_ENGINE})

  target_sources(${name}
    PRIVATE
    ${XR_ENGINE_SOURCES}
  )

  target_precompile_headers(${name}
    PRIVATE
    stdafx.h
  )

  target_compile_definitions(${name}
    PRIVATE
    ENGINE_BUILD
  )

  target_link_libraries(${name}
    PRIVATE
    discord
    dinput8
    dxsdk
    DPlay
    icu
    imgui
    luabind
    LuaJIT
    ReShadeCompat
    Vfw32
    XRay.Collision
    XRay.Core
    XRay.Game
    XRay.Engine
    XRay.Particles
    XRay.Render
    XRay.Render.API
    XRay.Sound
    XRay.XMLParser
  )

  set(PDB_ZIP ${name}_pdb.zip)
  add_custom_target(${name}-PDB)

  target_folder(${name}-PDB ${FOLDER_RELEASE})

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
configure_engine_target(AnomalyDX8 STATIC_RENDERER_R1 XRay.Render.R1)

add_engine_target(AnomalyDX8AVX)
enable_avx(AnomalyDX8AVX)
configure_engine_target(AnomalyDX8AVX STATIC_RENDERER_R1 XRay.Render.R1)

add_engine_target(AnomalyDX9)
configure_engine_target(AnomalyDX9 STATIC_RENDERER_R2 XRay.Render.R2)

add_engine_target(AnomalyDX9AVX)
enable_avx(AnomalyDX9AVX)
configure_engine_target(AnomalyDX9AVX STATIC_RENDERER_R2 XRay.Render.R2)

add_engine_target(AnomalyDX10)
configure_engine_target(AnomalyDX10 STATIC_RENDERER_R3 XRay.Render.R3)

add_engine_target(AnomalyDX10AVX)
enable_avx(AnomalyDX10AVX)
configure_engine_target(AnomalyDX10AVX STATIC_RENDERER_R3 XRay.Render.R3)

add_engine_target(AnomalyDX11)
configure_engine_target(AnomalyDX11 STATIC_RENDERER_R4 XRay.Render.R4)

add_engine_target(AnomalyDX11AVX)
enable_avx(AnomalyDX11AVX)
configure_engine_target(AnomalyDX11AVX STATIC_RENDERER_R4 XRay.Render.R4)
