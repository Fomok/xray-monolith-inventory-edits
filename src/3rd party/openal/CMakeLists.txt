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
  PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/OpenAL-Windows/Router"
)

enable_parallel_build(ode)
