include_guard()

# Add an XRay.Engine PDB target with the given name
macro(add_engine_pdb_impl name)
  set(PDB_ZIP ${name}_pdb.zip)
  add_custom_target(${name}-PDB)

  target_folder(${name}-PDB ${FOLDER_CI})
  target_sources(${name}-PDB PRIVATE ${PDB_ZIP})

  add_custom_command(
      OUTPUT ${PDB_ZIP}
      COMMAND ${CMAKE_COMMAND} -E tar "cf" "${PDB_ZIP}" --format=zip "$<TARGET_PDB_FILE:${name}>"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Zip $<TARGET_PDB_FILE:${name}> to ${CMAKE_BINARY_DIR}/${PDB_ZIP}."
      VERBATIM
  )

  add_dependencies(${name}-PDB ${name})
endmacro()

# Add a target with the given renderer, and an AVX variant
function(add_engine_target NAME)
  add_executable(${NAME} WIN32)

  if(${XRAY_AVX})
    set(${NAME}_NAME_EXECUTABLE ${NAME}.AVX)
  else()
    set(${NAME}_NAME_EXECUTABLE ${NAME})
  endif()

  string(REPLACE "." "" ${NAME}_NAME_OUTPUT ${${NAME}_NAME_EXECUTABLE})
  set_target_properties(${NAME}
    PROPERTIES OUTPUT_NAME
    ${${NAME}_NAME_OUTPUT}
  )
  target_folder(${NAME} ${FOLDER_EXECUTABLES})
  target_link_libraries(${NAME}
    PRIVATE
    XRay.Engine.Main
    ${ARGN}
  )
  add_engine_pdb_impl(${NAME})
endfunction()

# Setup executable targets
add_engine_target(
  Anomaly.DX8
  XRay.Render.R1
)
add_engine_target(
  Anomaly.DX9
  XRay.Render.R2
)
add_engine_target(
  Anomaly.DX10
  XRay.Render.R3
)
add_engine_target(
  Anomaly.DX11
  XRay.Render.R4
)

# Set visual studio startup project
set_property(
  DIRECTORY ${CMAKE_SOURCE_DIR}
  PROPERTY VS_STARTUP_PROJECT
  Anomaly.DX11
)
