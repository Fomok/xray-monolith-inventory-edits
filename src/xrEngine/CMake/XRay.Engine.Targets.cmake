include_guard()

# Add an XRay.Engine target with the given name
macro(add_engine_target_impl name)
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

# Enable AVX for the given target
macro(enable_avx TARGET)
  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /arch:AVX)
  endif()
endmacro()

# Add a target with the given renderer, and an AVX variant
macro(add_engine_target TARGET RENDERER)
  add_engine_target_impl(${TARGET})
  target_link_libraries(${TARGET} PRIVATE ${RENDERER})

  add_engine_target_impl(${TARGET}AVX)
  enable_avx(${TARGET}AVX)
  target_link_libraries(${TARGET}AVX PRIVATE ${RENDERER})
endmacro()

# Setup executable targets
add_engine_target(AnomalyDX8 XRay.Render.R1)
add_engine_target(AnomalyDX9 XRay.Render.R2)
add_engine_target(AnomalyDX10 XRay.Render.R3)
add_engine_target(AnomalyDX11 XRay.Render.R4)
