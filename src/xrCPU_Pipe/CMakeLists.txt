add_library(xrCPU_Pipe STATIC)

target_sources(xrCPU_Pipe PRIVATE
    PLC.cpp
    StdAfx.cpp
    ttapi.cpp
    xrCPU_Pipe.cpp
    xrSkin2W.cpp
    xrSkin2W_SSE.cpp
    xrSkin2W_thread.cpp
)

target_include_directories(xrCPU_Pipe
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_precompile_headers(xrCore
  PRIVATE stdafx.h
)

target_compile_definitions(xrCPU_Pipe
  PUBLIC
  NDEBUG
  XRCPU_PIPE_EXPORTS

  PRIVATE
  WIN32
  _WINDOWS
  _USRDLL
  _SECURE_SCL=0
)

enable_parallel_build(xrCPU_Pipe)
disable_exceptions()
silence_stdext_hash_deprecation_warnings(xrCPU_Pipe)
