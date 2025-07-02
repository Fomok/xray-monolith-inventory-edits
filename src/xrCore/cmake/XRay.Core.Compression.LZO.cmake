add_library(XRay.Core.Compression.LZO STATIC)

set_property(
  TARGET XRay.Core.Compression.LZO
  PROPERTY FOLDER
  ${FOLDER_XRAY_CORE}
)

target_sources(XRay.Core.Compression.LZO
  PRIVATE
  lzo_compressor.cpp
  rt_lzo1x_1.cpp
  rt_lzo1x_9x.cpp
  rt_lzo1x_d1.cpp
  rt_lzo1x_d2.cpp
  rt_lzo1x_d3.cpp
  rt_lzo_init.cpp
  
  lzo_compressor.h
  rt_config1x.h
  rt_lzo1x.h
  rt_lzoconf.h
  rt_lzodefs.h
  rt_lzo_conf.h
  rt_lzo_config.h
  rt_lzo_dict.h
  rt_lzo_ptr.h
  rt_miniacc.h
)

target_include_directories(XRay.Core.Compression.LZO
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core.Compression.LZO
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core.Compression.LZO
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core.Compression.LZO
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.Collision
)
