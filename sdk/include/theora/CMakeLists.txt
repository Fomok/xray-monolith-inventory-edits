add_library(libtheora_static STATIC)

target_sources(libtheora_static PRIVATE
  lib/analyze.c
  lib/apiwrapper.c
  lib/bitpack.c
  lib/cpu.c
  lib/decapiwrapper.c
  lib/decinfo.c
  lib/decode.c
  lib/dequant.c
  lib/encapiwrapper.c
  lib/encfrag.c
  lib/encinfo.c
  lib/encode.c
  #lib/encoder_disabled.c
  lib/enquant.c
  lib/fdct.c
  lib/fragment.c
  lib/huffdec.c
  lib/huffenc.c
  lib/idct.c
  lib/info.c
  lib/internal.c
  lib/mathops.c
  lib/mcenc.c
  lib/quant.c
  lib/rate.c
  lib/state.c
  lib/tokenize.c
  lib/x86_vc/mmxencfrag.c
  lib/x86_vc/mmxfdct.c
  lib/x86_vc/mmxfrag.c
  lib/x86_vc/mmxidct.c
  lib/x86_vc/mmxstate.c
  lib/x86_vc/x86enc.c
  lib/x86_vc/x86state.c

  codec.h
  theora.h
  theoradec.h
  lib/apiwrapper.h
  lib/bitpack.h
  lib/cpu.h
  lib/dct.h
  lib/decint.h
  lib/dequant.h
  lib/encint.h
  lib/enquant.h
  lib/huffdec.h
  lib/huffenc.h
  lib/huffman.h
  #lib/idct.h
  lib/internal.h
  lib/mathops.h
  lib/modedec.h
  lib/ocintrin.h
  lib/quant.h
)

target_include_directories(libtheora_static
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/sdk/include"
)

target_compile_definitions(libtheora_static
  PUBLIC
  NDEBUG

  PRIVATE
  WIN32
  _WINDOWS
  _CRT_SECURE_NO_DEPRECATE
  _CRT_NONSTDC_NO_DEPRECATE
  _BIND_TO_CURRENT_CRT_VERSION
)

enable_parallel_build(libtheora_static)
