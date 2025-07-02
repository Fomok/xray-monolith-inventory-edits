add_core_static(XRay.Core.Compression.PPMD
  PPMd.h
  PPMdType.h
  
  Model.cpp
  Coder.hpp
  
  SubAlloc.hpp
  
  compression_ppmd_stream.h
  compression_ppmd_stream_inline.h
  
  ppmd_compressor.cpp
  ppmd_compressor.h
)

target_folder(XRay.Core.Compression.PPMD ${FOLDER_XRAY_CORE_COMPRESSION})
