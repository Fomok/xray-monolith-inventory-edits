add_library(lua-extensions STATIC)

target_folder(lua-extensions ${FOLDER_LUA})

target_sources_sandboxed(lua-extensions
  DIRECTORY lua-extensions
  
  SOURCES
  base.cpp
  script_additional_libs.cpp
  script_additional_libs.h
)

target_include_directories(lua-extensions
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/lua-extensions
)

target_compile_options(lua-extensions
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/permissive->
  $<$<CXX_COMPILER_ID:MSVC>:/wd4005>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
  $<$<CXX_COMPILER_ID:Clang>:-Wno-macro-redefined>
  $<$<CXX_COMPILER_ID:Clang>:-Wno-deprecated-declarations>
)

target_compile_definitions(lua-extensions
  PRIVATE
  _CRT_SECURE_NO_WARNINGS
)

target_link_libraries(lua-extensions
  PRIVATE
  luafilesystem
  lua-marshal
  LuaPanda
  luasocket
  LuaJIT
  XRay.Core
)
