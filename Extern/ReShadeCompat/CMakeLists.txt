add_library(ReShadeCompat STATIC)

target_folder(ReShadeCompat ${FOLDER_EXTERNAL})

target_sources(ReShadeCompat PRIVATE
  addon.cpp
)

target_include_directories(ReShadeCompat
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/Extern
)

target_precompile_headers(ReShadeCompat
  PRIVATE
  stdafx.h
)

target_compile_definitions(ReShadeCompat
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
)