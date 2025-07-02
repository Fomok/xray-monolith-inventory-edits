add_core_static(XRay.Core.Memory.Shared
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

target_folder(XRay.Core.Memory.Shared ${FOLDER_XRAY_CORE_MEMORY})
