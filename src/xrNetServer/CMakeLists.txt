add_library(XRay.NetServer STATIC)

set_property(
  TARGET XRay.NetServer
  PROPERTY FOLDER
  ${FOLDER_XRAY_NET_SERVER}
)

target_sources(XRay.NetServer PRIVATE
  ip_filter.cpp
  NET_AuthCheck.cpp
  NET_Client.cpp
  NET_Common.cpp
  NET_Compressor.cpp
  NET_Log.cpp
  NET_Server.cpp

  ip_filter.h
  NET_AuthCheck.h
  NET_Client.h
  NET_Common.h
  NET_Compressor.h
  NET_Log.h
  NET_Messages.h
  NET_PlayersMonitor.h
  NET_Server.h
  NET_Shared.h
  stdafx.h
)

target_include_directories(XRay.NetServer
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.NetServer
  PRIVATE stdafx.h
)

target_link_libraries(XRay.NetServer
  PRIVATE
  DPlay
  DXERR
  Ws2_32
  XRay.Core
)
