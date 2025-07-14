add_library(serial INTERFACE)

target_folder(serial ${FOLDER_EXTERNAL})

target_sources(serial
  PRIVATE
  tbb/parallel_for.h
  tbb/tbb_annotate.h
)

target_include_directories(serial
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
)
