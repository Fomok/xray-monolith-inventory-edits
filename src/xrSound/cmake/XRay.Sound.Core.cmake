add_library(XRay.Sound.Core STATIC)

set_property(
  TARGET XRay.Sound.Core
  PROPERTY FOLDER
  ${FOLDER_XRAY_SOUND}
)

target_sources(XRay.Sound.Core
  PRIVATE
  SoundRender_Core.cpp
  SoundRender_CoreA.cpp
  SoundRender_Core_Processor.cpp
  SoundRender_Core_SourceManager.cpp
  SoundRender_Core_StartStop.cpp
  
  SoundRender_Core.h
  SoundRender_CoreA.h
)

target_precompile_headers(XRay.Sound.Core
  PRIVATE
  stdafx.h
)

target_include_directories(XRay.Sound.Core
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(XRay.Sound.Core
  PRIVATE
  XRSOUND_EXPORTS
)

target_link_libraries(XRay.Sound.Core
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
