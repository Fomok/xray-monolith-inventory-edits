add_module(XRay.Core.Memory.Shared
  PARENT XRay.Core

  SOURCES
  crc32.cpp
  mezz_stringbuffer.cpp
  xr_shared.cpp
  xrsharedmem.cpp
  xrstring.cpp
  
  mezz_stringbuffer.h
  xr_resource.h
  xr_shared.h
  xrsharedmem.h
  xrstring.h
)