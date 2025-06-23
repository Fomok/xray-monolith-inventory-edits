add_custom_target(xrEngine)

set_property(
  TARGET xrEngine
  PROPERTY FOLDER
  ${FOLDER_XRAY}
)

target_engine_sources(xrEngine ${XR_ENGINE_SOURCES})
