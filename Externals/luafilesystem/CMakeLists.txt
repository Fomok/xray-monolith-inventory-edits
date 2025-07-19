add_library(luafilesystem STATIC)

target_folder(luafilesystem ${FOLDER_LUA_EXTENSIONS})

set(LUAFILESYSTEM_DIR ../luafilesystem-git)

target_sources_sandboxed(luafilesystem
  DIRECTORY luafilesystem

  STRIP
  ${LUAFILESYSTEM_DIR}/src/

  SOURCES
  ${LUAFILESYSTEM_DIR}/src/lfs.c
  ${LUAFILESYSTEM_DIR}/src/lfs.h
)

target_include_directories(luafilesystem
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/luafilesystem
)

target_compile_definitions(luafilesystem
  PRIVATE
  _CRT_SECURE_NO_WARNINGS
)

target_link_libraries(luafilesystem
  PRIVATE
  LuaJIT
)