add_library(ogg STATIC)

target_folder(ogg ${FOLDER_EXTERNAL})

target_sources(ogg
  PRIVATE
  src/bitwise.c
  src/framing.c

  ogg.h
  os_types.h
)

target_include_directories(ogg
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${XRAY_EXTERNAL}
)
