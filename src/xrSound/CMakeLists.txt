add_library(xrSound STATIC)

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
    stdafx.cpp
)

target_precompile_headers(xrSound
  PRIVATE stdafx.h
)

target_include_directories(xrSound
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/sdk/include"
  "${CMAKE_SOURCE_DIR}/src/3rd party"
  "${CMAKE_SOURCE_DIR}/src/3rd party/openal/include"
)

target_compile_definitions(xrSound
  PUBLIC
  NDEBUG
  XRSOUND_EXPORTS

  PRIVATE
  WIN32
  _WINDOWS
  _USRDLL
  _SECURE_SCL=0
)

enable_parallel_build(xrSound)
disable_exceptions()
silence_stdext_hash_deprecation_warnings(xrSound)
