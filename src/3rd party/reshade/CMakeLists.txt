add_library(reshade INTERFACE)

set_property(
  TARGET reshade
  PROPERTY FOLDER
  ${FOLDER_3RD_PARTY}
)

target_include_directories(reshade
  INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
