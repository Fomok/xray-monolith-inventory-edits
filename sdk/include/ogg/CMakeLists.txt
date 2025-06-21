add_library(libogg_static STATIC)

target_sources(libogg_static PRIVATE
  src/bitwise.c
  src/framing.c
)

target_include_directories(libogg_static
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/sdk/include"
)

target_compile_definitions(libogg_static
  PUBLIC
  NDEBUG

  PRIVATE
  WIN32
  _WINDOWS
)

enable_parallel_build(libogg_static)
