add_module(X-Ray.Core.Threading
  PARENT XRay.Core

  SOURCES
  Lock.cpp
  ScopeLock.cpp
  xrSyncronize.cpp
  
  Lock.hpp
  ScopeLock.hpp
  xrSyncronize.h
)