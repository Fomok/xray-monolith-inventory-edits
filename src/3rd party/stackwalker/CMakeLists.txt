add_library(StackWalker INTERFACE)

target_folder(StackWalker ${FOLDER_EXTERNAL})

target_sources(StackWalker
  PRIVATE
  include/StackWalker.h
)

target_include_directories(StackWalker
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_library(
  LIBRARY_STACKWALKER StackWalker
  HINTS ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_link_libraries(StackWalker
  INTERFACE ${LIBRARY_STACKWALKER}
)
