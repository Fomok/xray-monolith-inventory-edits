add_library(XRay.Sound.Target STATIC)

target_folder(XRay.Sound.Target ${FOLDER_XRAY_SOUND})

target_sources(XRay.Sound.Target
  PRIVATE
  SoundRender_Target.cpp
  SoundRender_TargetA.cpp

  SoundRender_Target.h
  SoundRender_TargetA.h
)

target_precompile_headers(XRay.Sound.Target
  PRIVATE stdafx.h
)

target_include_directories(XRay.Sound.Target
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(XRay.Sound.Target
  PRIVATE
  XRSOUND_EXPORTS
)

target_link_libraries(XRay.Sound.Target
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
