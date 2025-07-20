add_library(optick STATIC)

target_folder(optick ${FOLDER_EXTERNAL})

target_compile_options(optick
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

target_compile_definitions(optick
  PUBLIC
  OPTICK_ENABLE_GPU=0
)

target_include_directories(optick
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/optick
)

set(OPTICK_DIR ../optick-git/src)

target_sources_sandboxed(optick
  DIRECTORY optick

  STRIP
  ${OPTICK_DIR}/

  SOURCES
  ${OPTICK_DIR}/optick.config.h
  ${OPTICK_DIR}/optick.h
  ${OPTICK_DIR}/optick_capi.cpp
  ${OPTICK_DIR}/optick_capi.h
  ${OPTICK_DIR}/optick_common.h
  ${OPTICK_DIR}/optick_core.cpp
  ${OPTICK_DIR}/optick_core.freebsd.h
  ${OPTICK_DIR}/optick_core.h
  ${OPTICK_DIR}/optick_core.linux.h
  ${OPTICK_DIR}/optick_core.macos.h
  ${OPTICK_DIR}/optick_core.platform.h
  ${OPTICK_DIR}/optick_core.win.h
  ${OPTICK_DIR}/optick_gpu.cpp
  ${OPTICK_DIR}/optick_gpu.d3d12.cpp
  ${OPTICK_DIR}/optick_gpu.h
  ${OPTICK_DIR}/optick_gpu.vulkan.cpp
  ${OPTICK_DIR}/optick_memory.h
  ${OPTICK_DIR}/optick_message.cpp
  ${OPTICK_DIR}/optick_message.h
  ${OPTICK_DIR}/optick_miniz.cpp
  ${OPTICK_DIR}/optick_miniz.h
  ${OPTICK_DIR}/optick_serialization.cpp
  ${OPTICK_DIR}/optick_serialization.h
  ${OPTICK_DIR}/optick_server.cpp
  ${OPTICK_DIR}/optick_server.h
)
