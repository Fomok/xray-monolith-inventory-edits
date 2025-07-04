add_module(XRay.Sound
  ROOT
  
  INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}

  PRECOMPILES stdafx.h

  DEFINES
  XRSOUND_EXPORTS

  LINKS
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

  SOURCES
  guids.cpp
  OpenALDeviceList.cpp
  sound.cpp

  cl_intersect.h
  OpenALDeviceList.h
  Sound.h
  SoundRender.h
  stdafx.h
)

include(XRay.Sound.Cache)
include(XRay.Sound.Core)
include(XRay.Sound.Emitter)
include(XRay.Sound.Environment)
include(XRay.Sound.Source)
include(XRay.Sound.Target)
