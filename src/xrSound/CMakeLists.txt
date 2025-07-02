add_library(XRay.Sound STATIC)

set_property(
  TARGET XRay.Sound
  PROPERTY FOLDER
  ${FOLDER_XRAY_SOUND}
)

target_sources(XRay.Sound PRIVATE
  guids.cpp
  OpenALDeviceList.cpp
  sound.cpp
  SoundRender_Cache.cpp
  SoundRender_Core.cpp
  SoundRender_CoreA.cpp
  SoundRender_Core_Processor.cpp
  SoundRender_Core_SourceManager.cpp
  SoundRender_Core_StartStop.cpp
  SoundRender_Emitter.cpp
  SoundRender_Emitter_FSM.cpp
  SoundRender_Emitter_StartStop.cpp
  SoundRender_Emitter_streamer.cpp
  SoundRender_Environment.cpp
  SoundRender_Source.cpp
  SoundRender_Source_loader.cpp
  SoundRender_Target.cpp
  SoundRender_TargetA.cpp

  cl_intersect.h
  OpenALDeviceList.h
  Sound.h
  SoundRender.h
  SoundRender_Cache.h
  SoundRender_Core.h
  SoundRender_CoreA.h
  SoundRender_Emitter.h
  SoundRender_Environment.h
  SoundRender_Source.h
  SoundRender_Target.h
  SoundRender_TargetA.h
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
)
