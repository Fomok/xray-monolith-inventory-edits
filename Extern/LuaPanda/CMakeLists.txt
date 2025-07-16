add_library(LuaPanda STATIC)

target_folder(LuaPanda ${FOLDER_LUA})

set(LUAPANDA_DIR ../LuaPanda-git)

target_sources(LuaPanda
  PRIVATE
  ${LUAPANDA_DIR}/Debugger/debugger_lib/libpdebug.cpp
  ${LUAPANDA_DIR}/Debugger/debugger_lib/libpdebug.h
)

target_include_directories(LuaPanda
  PUBLIC
  ${LUAPANDA_DIR}/Debugger/debugger_lib
)

target_compile_definitions(LuaPanda
  PRIVATE
  USE_SOURCE_CODE
)

target_link_libraries(LuaPanda
  PRIVATE
  LuaJIT
)