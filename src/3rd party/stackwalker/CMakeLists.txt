add_library(stackwalker INTERFACE)

set_property(
  TARGET stackwalker
  PROPERTY FOLDER
  ${FOLDER_3RD_PARTY}
)

target_sources(stackwalker
  PRIVATE
  include/StackWalker.h
)

target_include_directories(stackwalker
  INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
