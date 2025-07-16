add_library(luafilesystem STATIC)

target_folder(luafilesystem ${FOLDER_LUA})

set(LUAFILESYSTEM_DIR ../luafilesystem-git)

target_sources(luafilesystem
  PRIVATE
  ${LUAFILESYSTEM_DIR}/src/lfs.c
  ${LUAFILESYSTEM_DIR}/src/lfs.h
)

target_include_directories(luafilesystem
  PUBLIC
  ${LUAFILESYSTEM_DIR}/src
)

target_link_libraries(luafilesystem
  PRIVATE
  LuaJIT
)