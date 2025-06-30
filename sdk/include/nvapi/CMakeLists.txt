add_library(NVAPI INTERFACE)

set_property(
  TARGET NVAPI
  PROPERTY FOLDER
  ${FOLDER_SDK}
)

target_sources(NVAPI
  PRIVATE
  nvapi.h
  nvapi_lite_common.h
  nvapi_lite_d3dext.h
  nvapi_lite_salend.h
  nvapi_lite_salstart.h
  nvapi_lite_sli.h
  nvapi_lite_stereo.h
  nvapi_lite_surround.h
  NvApiDriverSettings.c
  NvApiDriverSettings.h
  nvHLSLExtns.h
  nvHLSLExtnsInternal.h
  nvShaderExtnEnums.h
)

target_include_directories(NVAPI
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

find_library(
  LIBRARY_NVAPI nvapi
  HINTS ${XRAY_SDK_LIB}
)

target_link_libraries(NVAPI
  INTERFACE
  ${LIBRARY_NVAPI}
)
