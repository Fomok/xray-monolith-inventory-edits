add_library(xrNetServer STATIC)

set_property(
  TARGET xrNetServer
  PROPERTY FOLDER
  ${FOLDER_XRAY}
)

target_sources(xrNetServer PRIVATE
  ip_filter.cpp
  NET_AuthCheck.cpp
  NET_Client.cpp
  NET_Common.cpp
  NET_Compressor.cpp
  NET_Log.cpp
  NET_Server.cpp
  stdafx.cpp

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

target_include_directories(xrNetServer
  PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_precompile_headers(xrNetServer
  PRIVATE stdafx.h
)

target_link_libraries(xrNetServer
  PUBLIC
  DPlay
  Ws2_32
  xrCore
)

enable_parallel_build(xrNetServer)
silence_stdext_hash_deprecation_warnings(xrNetServer)
