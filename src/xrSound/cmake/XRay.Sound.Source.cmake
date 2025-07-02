add_library(XRay.Sound.Source STATIC)

set_property(
  TARGET XRay.Sound.Source
  PROPERTY FOLDER
  ${FOLDER_XRAY_SOUND}
)

target_sources(XRay.Sound.Source
  PRIVATE
  SoundRender_Source.cpp
  SoundRender_Source_loader.cpp

  SoundRender_Source.h
)

target_precompile_headers(XRay.Sound.Source
  PRIVATE
  stdafx.h
)

target_include_directories(XRay.Sound.Source
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(XRay.Sound.Source
  PRIVATE
  XRSOUND_EXPORTS
)

target_link_libraries(XRay.Sound.Source
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
