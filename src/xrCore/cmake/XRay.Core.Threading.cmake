add_core_static(XRay.Core.Threading
  PRIVATE
  Lock.cpp
  ScopeLock.cpp
  xrSyncronize.cpp
  
  Lock.hpp
  ScopeLock.hpp
  xrSyncronize.h
)

target_folder(XRay.Core.Threading ${FOLDER_XRAY_CORE})
