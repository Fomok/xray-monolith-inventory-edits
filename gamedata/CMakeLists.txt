add_custom_target(Gamedata)

# Get a native version of our gamedata path
cmake_path(
  CONVERT ${CMAKE_SOURCE_DIR}/gamedata
  TO_NATIVE_PATH_LIST XRAY_GAMEDATA
)
set(
  XRAY_GAMEDATA ${XRAY_GAMEDATA}
  CACHE PATH "X-Ray Gamedata"
)

# Generated gamedata file
set(
  XRAY_GAMEDATA_SOURCE_NAME gamedata.db0
  CACHE PATH "X-Ray Gamedata DB Source Name"
)
set(
  XRAY_GAMEDATA_SOURCE_DB
  ${CMAKE_SOURCE_DIR}/${XRAY_GAMEDATA_SOURCE_NAME}
)

# Final gamedata file
set(
  XRAY_GAMEDATA_BINARY_NAME 00_modded_exes_gamedata.db0
  CACHE PATH "X-Ray Gamedata DB Binary Name"
)
set(
  XRAY_GAMEDATA_BINARY_DB
  ${CMAKE_CURRENT_BINARY_DIR}/${XRAY_GAMEDATA_BINARY_NAME}
)

# Add a custom command to generate our final .db0 file
add_custom_command(
  OUTPUT ${XRAY_GAMEDATA_BINARY_DB}
  COMMAND ${XRAY_COMPRESSOR}
    ${XRAY_GAMEDATA}
    -ltx build_patch.ltx
    -pack
    -strong
    -1024
    -db
  COMMAND ${CMAKE_COMMAND} -E rm -f ${XRAY_GAMEDATA_BINARY_DB}
  COMMAND ${CMAKE_COMMAND} -E rename
    ${XRAY_GAMEDATA_SOURCE_DB} ${XRAY_GAMEDATA_BINARY_DB}
  BYPRODUCTS ${XRAY_COMPRESSOR_LOG}
  WORKING_DIRECTORY ${XRAY_COMPRESSOR_DIR}
  VERBATIM
)

target_sources(Gamedata PRIVATE ${XRAY_GAMEDATA_BINARY_NAME})

# Glob source files so they display in IDEs
file(GLOB_RECURSE SOURCES_GAMEDATA *)
target_sources(Gamedata PRIVATE ${SOURCES_GAMEDATA})
