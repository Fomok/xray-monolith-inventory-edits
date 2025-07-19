add_library(StackWalker STATIC)

target_folder(StackWalker ${FOLDER_EXTERNAL})

set(STACKWALKER_DIR ../StackWalker-git/Main/StackWalker)

target_sources_sandboxed(StackWalker
  DIRECTORY StackWalker

  STRIP
  ${STACKWALKER_DIR}/

  SOURCES
  ${STACKWALKER_DIR}/StackWalker.cpp
  ${STACKWALKER_DIR}/StackWalker.h
)

target_include_directories(StackWalker
  INTERFACE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/StackWalker
)

target_compile_options(StackWalker
  PRIVATE
  $<$<CXX_COMPILER_ID:Clang>:-Wno-switch>
)