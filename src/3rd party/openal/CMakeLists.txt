add_library(OpenAL32 STATIC)

set_property(
  TARGET OpenAL32
  PROPERTY FOLDER
  ${FOLDER_3RD_PARTY}
)

target_sources(OpenAL32 PRIVATE
  OpenAL-Windows/Router/al.cpp
  OpenAL-Windows/Router/alc.cpp
  OpenAL-Windows/Router/alList.cpp
  OpenAL-Windows/Router/OpenAL32.cpp

  OpenAL-Windows/Router/alList.h
  OpenAL-Windows/Router/OpenAL32.h
  OpenAL-Windows/Router/resource.h
)

target_include_directories(OpenAL32
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/OpenAL-Windows/Router"
)

target_compile_definitions(OpenAL32
  PUBLIC
  NDEBUG

  PRIVATE
  WIN32
  _WINDOWS
  _SECURE_SCL=0
)

enable_parallel_build(ode)
