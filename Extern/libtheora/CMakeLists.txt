add_library(libtheora STATIC)

target_folder(libtheora ${FOLDER_EXTERNAL})

target_sources_sandboxed(libtheora
  DIRECTORY theora
  SOURCES
  lib/x86/mmxencfrag.c
  lib/x86/mmxfdct.c
  lib/x86/mmxfrag.c
  lib/x86/mmxfrag.h
  lib/x86/mmxidct.c
  lib/x86/mmxloop.h
  lib/x86/mmxstate.c
  lib/x86/sse2fdct.c
  lib/x86/x86enc.c
  lib/x86/x86enc.h
  lib/x86/x86int.h
  lib/x86/x86state.c
  
  lib/x86_vc/mmxencfrag.c
  lib/x86_vc/mmxfdct.c
  lib/x86_vc/mmxfrag.c
  lib/x86_vc/mmxfrag.h
  lib/x86_vc/mmxidct.c
  lib/x86_vc/mmxloop.h
  lib/x86_vc/mmxstate.c
  lib/x86_vc/x86enc.c
  lib/x86_vc/x86enc.h
  lib/x86_vc/x86int.h
  lib/x86_vc/x86state.c

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

  codec.h
  theora.h
  theoraenc.h
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

target_include_directories(libtheora
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/lib/x86
)

target_link_libraries(libtheora
  PRIVATE
  libogg
)