add_library(LZO STATIC)

target_folder(LZO ${FOLDER_EXTERNAL})

set(LZO_DIR ../LZO-xr)

target_compile_options(LZO
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/permissive->
  $<$<CXX_COMPILER_ID:MSVC>:/wd5033>
)

target_include_directories(LZO
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/LZO
)

target_sources_sandboxed(LZO
  DIRECTORY LZO

  STRIP
  ${LZO_DIR}/
  
  SOURCES
  ${LZO_DIR}/rt_config1x.h
  ${LZO_DIR}/rt_lzo_conf.h
  ${LZO_DIR}/rt_lzo_config.h
  ${LZO_DIR}/rt_lzo_dict.h
  ${LZO_DIR}/rt_lzo_dll.ch
  ${LZO_DIR}/rt_lzo_init.cpp
  ${LZO_DIR}/rt_lzo_ptr.h
  ${LZO_DIR}/rt_lzo_swd.ch
  ${LZO_DIR}/rt_lzo_ptr.h
  ${LZO_DIR}/rt_lzo_swd.ch
  ${LZO_DIR}/rt_lzo_mchw.ch
  ${LZO_DIR}/rt_lzo1_d.ch
  ${LZO_DIR}/rt_lzo1x_1.cpp
  ${LZO_DIR}/rt_lzo1x_9x.cpp
  ${LZO_DIR}/rt_lzo1x_c.ch
  ${LZO_DIR}/rt_lzo1x_d.ch
  ${LZO_DIR}/rt_lzo1x_d1.cpp
  ${LZO_DIR}/rt_lzo1x_d2.cpp
  ${LZO_DIR}/rt_lzo1x_d3.cpp
  ${LZO_DIR}/rt_lzo1x.h
  ${LZO_DIR}/rt_lzoconf.h
  ${LZO_DIR}/rt_lzodefs.h
  ${LZO_DIR}/rt_miniacc.h
)
