add_library(reshade INTERFACE)

target_folder(reshade ${FOLDER_SDK})

set(RESHADE_DIR ../reshade-git)

target_sources_sandboxed(reshade
  DIRECTORY reshade

  STRIP
  ${RESHADE_DIR}/include/

  SOURCES
  ${RESHADE_DIR}/include/reshade.hpp
  ${RESHADE_DIR}/include/reshade_api.hpp
  ${RESHADE_DIR}/include/reshade_api_device.hpp
  ${RESHADE_DIR}/include/reshade_api_format.hpp
  ${RESHADE_DIR}/include/reshade_api_pipeline.hpp
  ${RESHADE_DIR}/include/reshade_api_resource.hpp
  ${RESHADE_DIR}/include/reshade_events.hpp
  ${RESHADE_DIR}/include/reshade_overlay.hpp
)

target_include_directories(reshade
  INTERFACE
  ${CMAKE_CURRENT_BINARY_DIR}
)
