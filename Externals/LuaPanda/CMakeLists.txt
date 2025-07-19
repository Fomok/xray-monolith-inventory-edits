add_library(LuaPanda STATIC)

target_folder(LuaPanda ${FOLDER_LUA_EXTENSIONS})

set(LUAPANDA_DIR ../LuaPanda-git/Debugger/debugger_lib)

target_sources_sandboxed(LuaPanda
  DIRECTORY LuaPanda

  STRIP
  ${LUAPANDA_DIR}/

  SOURCES
  ${LUAPANDA_DIR}/libpdebug.cpp
  ${LUAPANDA_DIR}/libpdebug.h
)

target_include_directories(LuaPanda
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/LuaPanda
)

target_compile_options(LuaPanda
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4267>
)

target_compile_definitions(LuaPanda
  PRIVATE
  USE_SOURCE_CODE
)

target_link_libraries(LuaPanda
  PRIVATE
  LuaJIT
)