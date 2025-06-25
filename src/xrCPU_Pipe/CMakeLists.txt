add_library(xrCPU_Pipe STATIC)

set_property(
  TARGET xrCPU_Pipe
  PROPERTY FOLDER
  ${FOLDER_XRAY}
)

target_sources_grouped(
  TARGET xrCPU_Pipe
  NAME "Kernel/Source Files"
  FILES
  StdAfx.cpp
  xrCPU_Pipe.cpp
)

target_sources_grouped(
  TARGET xrCPU_Pipe
  NAME "Kernel/Header Files"
  FILES
  StdAfx.h
  xrCPU_Pipe.h
)

target_sources_grouped(
  TARGET xrCPU_Pipe
  NAME "PLC"
  FILES
  PLC.cpp
)

target_sources_grouped(
  TARGET xrCPU_Pipe
  NAME "Resources"
  FILES
  resource.h
  xrCPU_Pipe.rc
)

target_sources_grouped(
  TARGET xrCPU_Pipe
  NAME "Skinning"
  FILES
  xrSkin2W.cpp
  xrSkin2W_SSE.cpp
  xrSkin2W_thread.cpp
)

target_sources_grouped(
  TARGET xrCPU_Pipe
  NAME "TTAPI/Source Files"
  FILES
  ttapi.cpp
)

target_sources_grouped(
  TARGET xrCPU_Pipe
  NAME "TTAPI/Header Files"
  FILES
  ttapi.h
)

target_include_directories(xrCPU_Pipe
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(xrCPU_Pipe
  PRIVATE stdafx.h
)

target_link_libraries(xrCPU_Pipe
  PUBLIC
  xrCDB
)
