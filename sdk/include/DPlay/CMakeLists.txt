add_library(DPlay INTERFACE)

set_property(
  TARGET DPlay
  PROPERTY FOLDER
  ${FOLDER_SDK}
)

target_sources(DPlay
  PRIVATE
  dpaddr.h
  dplay8.h
)

target_include_directories(DPlay
  INTERFACE
  ${XRAY_SDK_INCLUDE}
)
