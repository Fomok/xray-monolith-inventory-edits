add_library(OpenAL32 STATIC)

target_folder(OpenAL32 ${FOLDER_EXTERNAL})

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
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/OpenAL-Windows/Router
)
