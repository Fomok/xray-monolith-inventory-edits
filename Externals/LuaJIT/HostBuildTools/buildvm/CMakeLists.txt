cmake_minimum_required(VERSION 3.22)

project(buildvm C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_VERBOSE_MAKEFILE)
  set(CMAKE_EXECUTE_PROCESS_COMMAND_ECHO STDOUT)
endif()

separate_arguments(HOST_ACFLAGS)

add_executable(buildvm)

target_folder(buildvm ${FOLDER_LUA_BUILD})

target_sources(buildvm PRIVATE
  "${BUILDVM_LUAJIT_DIR}/host/buildvm_asm.c"
  "${BUILDVM_LUAJIT_DIR}/host/buildvm_fold.c"
  "${BUILDVM_LUAJIT_DIR}/host/buildvm_lib.c"
  #"${BUILDVM_LUAJIT_DIR}/host/buildvm_libbc.h"
  "${BUILDVM_LUAJIT_DIR}/host/buildvm_peobj.c"
  "${BUILDVM_LUAJIT_DIR}/host/buildvm.c"
  "${BUILDVM_LUAJIT_DIR}/host/buildvm.h"
  "${BUILDVM_ARCH}"
)

add_dependencies(buildvm buildvm_arch)

target_include_directories(buildvm
  PRIVATE
  ${HOST_INCLUDES}
  ${BUILDVM_ARCH_DIR}
  ${BUILDVM_LUAJIT_DIR}
)

target_compile_options(buildvm
  PRIVATE
  ${HOST_ACFLAGS}
  $<$<CXX_COMPILER_ID:MSVC>:/wd4057>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4127>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4310>
  $<$<CXX_COMPILER_ID:Clang>:-Wno-incompatible-pointer-types>
)

target_compile_definitions(buildvm
  PRIVATE
  _CRT_SECURE_NO_WARNINGS
)