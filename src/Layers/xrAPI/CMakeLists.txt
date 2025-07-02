add_library(xrAPI STATIC)

set_property(
  TARGET xrAPI
  PROPERTY FOLDER
  ${FOLDER_RENDER}
)

target_sources(xrAPI
  PRIVATE
  xrAPI.cpp
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
  PRIVATE
  xrRender
)
