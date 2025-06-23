add_library(serial INTERFACE)

set_property(
  TARGET serial
  PROPERTY FOLDER
  ${FOLDER_3RD_PARTY}
)

target_sources(serial
  PRIVATE
  tbb/parallel_for.h
  tbb/tbb_annotate.h
)

target_include_directories(serial
  INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
