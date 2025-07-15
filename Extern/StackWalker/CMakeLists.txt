add_library(StackWalker STATIC)

target_folder(StackWalker ${FOLDER_EXTERNAL})

target_sources(StackWalker
  PRIVATE
  ../StackWalker-git/Main/StackWalker/StackWalker.cpp
  ../StackWalker-git/Main/StackWalker/StackWalker.h
)

target_include_directories(StackWalker
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/../StackWalker-git/Main/StackWalker
)