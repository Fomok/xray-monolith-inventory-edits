
add_core_static(XRay.Core.Compression.LZO
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
target_folder(XRay.Core.Compression.LZO ${FOLDER_XRAY_CORE_COMPRESSION})
