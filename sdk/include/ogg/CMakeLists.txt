add_library(ogg STATIC)

set_property(
  TARGET ogg
  PROPERTY FOLDER
  ${FOLDER_SDK}
)

target_sources(ogg PRIVATE
  src/bitwise.c
  src/framing.c

  ogg.h
  os_types.h
)

target_include_directories(ogg
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/sdk/include
)
