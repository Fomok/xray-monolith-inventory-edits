add_library(libogg STATIC)

target_folder(libogg ${FOLDER_EXTERNAL})

set(LIBOGG_DIR ../libogg-git)

target_sources_sandboxed(libogg
  STRIP
  ${LIBOGG_DIR}/src/
  ${LIBOGG_DIR}/include/

  SOURCES
  ${LIBOGG_DIR}/src/bitwise.c
  ${LIBOGG_DIR}/src/framing.c

  ${LIBOGG_DIR}/include/ogg/ogg.h
  ${LIBOGG_DIR}/include/ogg/os_types.h
)

target_include_directories(libogg
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(libogg
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/wd4456>
)
