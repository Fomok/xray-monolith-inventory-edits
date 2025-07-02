add_library(XRay.Sound.Environment STATIC)

set_property(
  TARGET XRay.Sound.Environment
  PROPERTY FOLDER
  ${FOLDER_XRAY_SOUND}
)

target_sources(XRay.Sound.Environment
  PRIVATE
  SoundRender_Environment.cpp
  SoundRender_Environment.h
)

target_precompile_headers(XRay.Sound.Environment PRIVATE stdafx.h)

target_include_directories(XRay.Sound.Environment
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(XRay.Sound.Environment
  PRIVATE
  XRSOUND_EXPORTS
)

target_link_libraries(XRay.Sound.Environment
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
