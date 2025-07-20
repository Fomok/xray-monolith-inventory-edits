cmake_minimum_required(VERSION 3.22)

project(minilua C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_VERBOSE_MAKEFILE)
  set(CMAKE_EXECUTE_PROCESS_COMMAND_ECHO STDOUT)
endif()

separate_arguments(HOST_ACFLAGS)

add_executable(minilua)

target_folder(minilua ${FOLDER_LUA_BUILD})

target_sources(minilua PRIVATE
  "${LUAJIT_DIR}/host/minilua.c"
)

target_compile_options(minilua
  PRIVATE
  ${HOST_ACFLAGS}
  $<$<CXX_COMPILER_ID:MSVC>:/wd4132>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4324>
)

target_compile_definitions(minilua
  PRIVATE
  _CRT_SECURE_NO_WARNINGS
)

target_include_directories(minilua
  PRIVATE
  ${HOST_INCLUDES}
)

target_link_libraries(minilua
  PRIVATE
  $<$<BOOL:${LUA_USE_POSIX}>:m>
)
