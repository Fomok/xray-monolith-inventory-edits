add_library(DPlay INTERFACE)
target_folder(DPlay ${FOLDER_SDK})

target_sources(DPlay
  PRIVATE
  dpaddr.h
  dplay8.h
)

target_include_directories(DPlay
  INTERFACE
  ${XRAY_SDK_INCLUDE}
)
