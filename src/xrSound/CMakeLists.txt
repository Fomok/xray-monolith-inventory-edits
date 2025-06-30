add_library(xrSound STATIC)

set_property(
  TARGET xrSound
  PROPERTY FOLDER
  ${FOLDER_XRAY}
)

target_sources(xrSound PRIVATE
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

target_precompile_headers(xrSound PRIVATE stdafx.h)

target_include_directories(xrSound
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(xrSound
  PRIVATE
  XRSOUND_EXPORTS
)

target_link_libraries(xrSound
  PUBLIC
  dxsdk
  dxguid
  ogg
  theora
  vorbis
  vorbisfile
  OpenAL32
  tbb
  xrCore
)
