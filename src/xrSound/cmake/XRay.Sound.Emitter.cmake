add_library(XRay.Sound.Emitter STATIC)

target_folder(XRay.Sound.Emitter ${FOLDER_XRAY_SOUND})

target_sources(XRay.Sound.Emitter
  PRIVATE
  SoundRender_Emitter.cpp
  SoundRender_Emitter_FSM.cpp
  SoundRender_Emitter_StartStop.cpp
  SoundRender_Emitter_streamer.cpp

  SoundRender_Emitter.h
)

target_precompile_headers(XRay.Sound.Emitter
  PRIVATE
  stdafx.h
)

target_include_directories(XRay.Sound.Emitter
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(XRay.Sound.Emitter
  PRIVATE
  XRSOUND_EXPORTS
)

target_link_libraries(XRay.Sound.Emitter
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
