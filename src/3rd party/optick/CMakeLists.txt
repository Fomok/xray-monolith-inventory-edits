add_library(optick STATIC)

set_property(
  TARGET optick
  PROPERTY FOLDER
  ${FOLDER_3RD_PARTY}
)

target_sources(optick
  PRIVATE
  ../optick-git/src/optick.config.h
  ../optick-git/src/optick.h
  ../optick-git/src/optick_capi.cpp
  ../optick-git/src/optick_capi.h
  ../optick-git/src/optick_common.h
  ../optick-git/src/optick_core.cpp
  ../optick-git/src/optick_core.freebsd.h
  ../optick-git/src/optick_core.h
  ../optick-git/src/optick_core.linux.h
  ../optick-git/src/optick_core.macos.h
  ../optick-git/src/optick_core.platform.h
  ../optick-git/src/optick_core.win.h
  ../optick-git/src/optick_gpu.cpp
  ../optick-git/src/optick_gpu.d3d12.cpp
  ../optick-git/src/optick_gpu.h
  ../optick-git/src/optick_gpu.vulkan.cpp
  ../optick-git/src/optick_memory.h
  ../optick-git/src/optick_message.cpp
  ../optick-git/src/optick_message.h
  ../optick-git/src/optick_miniz.cpp
  ../optick-git/src/optick_miniz.h
  ../optick-git/src/optick_serialization.cpp
  ../optick-git/src/optick_serialization.h
  ../optick-git/src/optick_server.cpp
  ../optick-git/src/optick_server.h
)

target_include_directories(optick
  PUBLIC
  "${CMAKE_SOURCE_DIR}/src/3rd party/optick-git/src"
)

target_compile_definitions(optick
  PUBLIC
  OPTICK_ENABLE_GPU_VULKAN=0
)
