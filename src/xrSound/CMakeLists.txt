include(cmake/XRay.Sound.Cache.cmake)
include(cmake/XRay.Sound.Core.cmake)
include(cmake/XRay.Sound.Emitter.cmake)
include(cmake/XRay.Sound.Environment.cmake)
include(cmake/XRay.Sound.Source.cmake)
include(cmake/XRay.Sound.Target.cmake)

add_library(XRay.Sound STATIC)

target_folder(XRay.Sound ${FOLDER_XRAY_SOUND})

target_sources(XRay.Sound PRIVATE
  guids.cpp
  OpenALDeviceList.cpp
  sound.cpp

  cl_intersect.h
  OpenALDeviceList.h
  Sound.h
  SoundRender.h
  stdafx.h
)

target_precompile_headers(XRay.Sound PRIVATE stdafx.h)

target_include_directories(XRay.Sound
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(XRay.Sound
  PRIVATE
  XRSOUND_EXPORTS
)

target_link_libraries(XRay.Sound
  PRIVATE
  dxsdk
  dxguid
  ogg
  theora
  vorbis
  vorbisfile
  OpenAL32
  tbb
  XRay.Core
  XRay.Render
  XRay.Render.API
  XRay.Sound.Cache
  XRay.Sound.Core
  XRay.Sound.Emitter
  XRay.Sound.Environment
  XRay.Sound.Source
  XRay.Sound.Target
)
