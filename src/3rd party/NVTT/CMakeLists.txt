add_library(NVTT STATIC)

target_sources(NVTT PRIVATE
    src/nvcore/Debug.cpp
    src/nvcore/Library.cpp
    src/nvcore/Memory.cpp
    src/nvcore/poshlib/posh.c
    src/nvcore/Radix.cpp
    src/nvcore/StrLib.cpp
    src/nvcore/TextReader.cpp
    src/nvcore/TextWriter.cpp
    src/nvcore/Tokenizer.cpp
    src/nvimage/BlockDXT.cpp
    src/nvimage/ColorBlock.cpp
    src/nvimage/ConeMap.cpp
    src/nvimage/DirectDrawSurface.cpp
    src/nvimage/Filter.cpp
    src/nvimage/FloatImage.cpp
    src/nvimage/HoleFilling.cpp
    src/nvimage/Image.cpp
    src/nvimage/ImageIO.cpp
    src/nvimage/NormalMap.cpp
    src/nvimage/NormalMipmap.cpp
    src/nvimage/Quantize.cpp
    src/nvmath/Basis.cpp
    src/nvmath/Montecarlo.cpp
    src/nvmath/Plane.cpp
    src/nvmath/Random.cpp
    src/nvmath/SphericalHarmonic.cpp
    src/nvmath/Triangle.cpp
    src/nvmath/TriBox.cpp
    src/nvtt/CompressDXT.cpp
    src/nvtt/CompressionOptions.cpp
    src/nvtt/Compressor.cpp
    src/nvtt/CompressRGB.cpp
    src/nvtt/cuda/CudaCompressDXT.cpp
    src/nvtt/cuda/CudaUtils.cpp
    src/nvtt/InputOptions.cpp
    src/nvtt/nvtt.cpp
    src/nvtt/nvtt_wrapper.cpp
    src/nvtt/OptimalCompressDXT.cpp
    src/nvtt/OutputOptions.cpp
    src/nvtt/QuickCompressDXT.cpp
    src/nvtt/squish/alpha.cpp
    src/nvtt/squish/clusterfit.cpp
    src/nvtt/squish/colourblock.cpp
    src/nvtt/squish/colourfit.cpp
    src/nvtt/squish/colourset.cpp
    src/nvtt/squish/fastclusterfit.cpp
    src/nvtt/squish/maths.cpp
    src/nvtt/squish/rangefit.cpp
    src/nvtt/squish/singlechannelfit.cpp
    src/nvtt/squish/singlecolourfit.cpp
    src/nvtt/squish/squish.cpp
    src/nvtt/squish/weightedclusterfit.cpp
)

target_include_directories(NVTT
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/nvtt/squish"
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_definitions(NVTT
  PUBLIC
  NDEBUG

  PRIVATE
  WIN32
  _MBCS
)

enable_parallel_build(NVTT)
