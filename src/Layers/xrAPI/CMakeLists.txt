add_library(xrAPI STATIC)

set_property(
  TARGET xrAPI
  PROPERTY FOLDER
  ${FOLDER_RENDER}
)

target_sources_grouped(
  TARGET xrAPI
  NAME "Kernel/Source Files"
  FILES
  xrAPI.cpp
)

target_sources_grouped(
  TARGET xrAPI
  NAME "Kernel/Header Files"
  FILES
  stdafx.h
  xrAPI.h
)

target_include_directories(xrAPI
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(xrAPI
  PRIVATE stdafx.h
)

target_link_libraries(xrAPI
  PUBLIC
  xrRender
)
