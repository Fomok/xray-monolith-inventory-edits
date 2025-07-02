add_library(XRay.Core.FS STATIC)

target_folder(XRay.Core.FS ${FOLDER_XRAY_CORE})

target_sources(XRay.Core.FS
  PRIVATE
  file_stream_reader.cpp
  
  file_stream_reader.h
  
  stream_reader.cpp
  
  stream_reader.h
  stream_reader_inline.h
  
  FileSystem.cpp
  FileSystem_borland.cpp
  FS.cpp
  LocatorAPI.cpp
  LocatorAPI_auth.cpp
  LocatorAPI_defs.cpp
  #LocatorAPI_Notifications.cpp
  log.cpp
  NET_utils.cpp
  Xr_ini.cpp
  
  FileSystem.h
  FS.h
  FS_impl.h
  FS_internal.h
  LocatorAPI.h
  LocatorAPI_defs.h
  #LocatorAPI_Notifications.h
  log.h
  net_utils.h
  xr_ini.h
)

target_include_directories(XRay.Core.FS
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core.FS
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core.FS
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core.FS
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.Collision
)
