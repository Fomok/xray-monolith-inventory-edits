add_library(XRay.Sound.Cache STATIC)

set_property(
  TARGET XRay.Sound.Cache
  PROPERTY FOLDER
  ${FOLDER_XRAY_SOUND}
)

target_sources(XRay.Sound.Cache
  PRIVATE
  SoundRender_Cache.cpp
  SoundRender_Cache.h
)

target_precompile_headers(XRay.Sound.Cache
  PRIVATE stdafx.h
)

target_include_directories(XRay.Sound.Cache
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(XRay.Sound.Cache
  PRIVATE
  XRSOUND_EXPORTS
)

target_link_libraries(XRay.Sound.Cache
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
)
