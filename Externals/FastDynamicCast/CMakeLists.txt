add_library(FastDynamicCast INTERFACE)

target_folder(FastDynamicCast ${FOLDER_EXTERNAL})

set(FAST_DYNAMIC_CAST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../FastDynamicCast-git)

target_include_directories(FastDynamicCast
  INTERFACE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/FastDynamicCast
)

target_sources_sandboxed(FastDynamicCast
  DIRECTORY FastDynamicCast
  STRIP
  ${FAST_DYNAMIC_CAST_DIR}/
  SOURCES
  ${FAST_DYNAMIC_CAST_DIR}/fast_dynamic_cast.h
)