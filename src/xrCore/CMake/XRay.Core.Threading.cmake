add_module(XRay.Core.Threading
  CHILD_OF XRay.Core

  SOURCES
  Lock.cpp
  ScopeLock.cpp
  xrSyncronize.cpp
  
  Lock.hpp
  ScopeLock.hpp
  xrSyncronize.h
)