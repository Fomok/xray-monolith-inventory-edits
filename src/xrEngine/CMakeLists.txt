add_library(xrEngine INTERFACE)

set_property(
  TARGET xrEngine
  PROPERTY FOLDER
  ${FOLDER_XRAY}
)

target_sources(xrEngine
  PRIVATE
  ${XR_ENGINE_SOURCES}
)

target_include_directories(xrEngine
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
)
