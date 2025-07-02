add_library(reshade INTERFACE)

target_folder(reshade ${FOLDER_EXTERNAL})

target_include_directories(reshade
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
)
