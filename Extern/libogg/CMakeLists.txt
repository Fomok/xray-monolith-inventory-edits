add_library(libogg STATIC)

target_folder(libogg ${FOLDER_EXTERNAL})

target_sources_sandboxed(libogg
  DIRECTORY ogg
  SOURCES
  src/bitwise.c
  src/framing.c

  ogg.h
  os_types.h
)

target_include_directories(libogg
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)
