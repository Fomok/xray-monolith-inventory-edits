add_library(libogg_static STATIC)

set_property(
  TARGET libogg_static
  PROPERTY FOLDER
  ${FOLDER_SDK}
)

target_sources(libogg_static PRIVATE
  src/bitwise.c
  src/framing.c

  ogg.h
  os_types.h
)

target_include_directories(libogg_static
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/sdk/include"
)

enable_parallel_build(libogg_static)
