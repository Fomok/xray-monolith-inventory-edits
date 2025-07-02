add_library(XRay.CPUPipe STATIC)

set_property(
  TARGET XRay.CPUPipe
  PROPERTY FOLDER
  ${FOLDER_XRAY_CPU_PIPE}
)

target_sources(XRay.CPUPipe
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

target_include_directories(XRay.CPUPipe
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.CPUPipe
  PRIVATE stdafx.h
)

target_link_libraries(XRay.CPUPipe
  PRIVATE
  XRay.Render.API
  XRay.CDB
  XRay.Render
)
