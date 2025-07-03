add_module(XRay.Core.Compression.PPMD
  PARENT XRay.Core
  
  SOURCES
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