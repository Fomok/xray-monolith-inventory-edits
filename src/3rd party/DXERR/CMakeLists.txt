add_library(DXERR STATIC)

set_property(
  TARGET DXERR
  PROPERTY FOLDER
  ${FOLDER_3RD_PARTY}
)

target_sources(DXERR PRIVATE
  dxerr.h
  dxerr.cpp
)

target_include_directories(DXERR
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

enable_parallel_build(DXERR)
silence_stdext_hash_deprecation_warnings(DXERR)
