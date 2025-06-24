add_library(StackWalker INTERFACE)

set_property(
  TARGET StackWalker
  PROPERTY FOLDER
  ${FOLDER_3RD_PARTY}
)

target_sources(StackWalker
  PRIVATE
  include/StackWalker.h
)

target_include_directories(StackWalker
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_library(
  STACKWALKER_LIBRARY StackWalker
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_link_libraries(StackWalker
  INTERFACE ${STACKWALKER_LIBRARY}
)
