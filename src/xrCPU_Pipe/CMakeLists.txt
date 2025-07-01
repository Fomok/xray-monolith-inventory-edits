add_library(xrCPU_Pipe STATIC)

set_property(
  TARGET xrCPU_Pipe
  PROPERTY FOLDER
  ${FOLDER_XRAY_CPU_PIPE}
)

target_sources(xrCPU_Pipe
  PRIVATE
  xrCPU_Pipe.cpp
  
  StdAfx.h
  xrCPU_Pipe.h
  
  PLC.cpp
  
  resource.h
  xrCPU_Pipe.rc
  
  xrSkin2W.cpp
  xrSkin2W_SSE.cpp
  xrSkin2W_thread.cpp
  
  ttapi.cpp
  
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
  PRIVATE
  xrAPI
  xrCDB
  xrRender
)
