add_library(DXERR STATIC)

target_folder(DXERR ${FOLDER_EXTERNAL})

target_sources(DXERR PRIVATE
  dxerr.h
  dxerr.cpp
)

target_include_directories(DXERR
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)
