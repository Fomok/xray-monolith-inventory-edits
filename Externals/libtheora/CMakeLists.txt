add_library(libtheora STATIC)

target_folder(libtheora ${FOLDER_EXTERNAL})

set(LIBTHEORA_DIR ../libtheora-git)

target_sources_sandboxed(libtheora
  DIRECTORY theora
  
  STRIP
  ${LIBTHEORA_DIR}/lib/
  ${LIBTHEORA_DIR}/include/theora/

  SOURCES
  ${LIBTHEORA_DIR}/include/theora/codec.h
  ${LIBTHEORA_DIR}/include/theora/theora.h
  ${LIBTHEORA_DIR}/include/theora/theoraenc.h
  ${LIBTHEORA_DIR}/include/theora/theoradec.h
  
  ${LIBTHEORA_DIR}/lib/x86/mmxencfrag.c
  ${LIBTHEORA_DIR}/lib/x86/mmxfdct.c
  ${LIBTHEORA_DIR}/lib/x86/mmxfrag.c
  ${LIBTHEORA_DIR}/lib/x86/mmxfrag.h
  ${LIBTHEORA_DIR}/lib/x86/mmxidct.c
  ${LIBTHEORA_DIR}/lib/x86/mmxloop.h
  ${LIBTHEORA_DIR}/lib/x86/mmxstate.c
  ${LIBTHEORA_DIR}/lib/x86/sse2fdct.c
  ${LIBTHEORA_DIR}/lib/x86/x86enc.c
  ${LIBTHEORA_DIR}/lib/x86/x86enc.h
  ${LIBTHEORA_DIR}/lib/x86/x86int.h
  ${LIBTHEORA_DIR}/lib/x86/x86state.c

  ${LIBTHEORA_DIR}/lib/x86_vc/mmxencfrag.c
  ${LIBTHEORA_DIR}/lib/x86_vc/mmxfdct.c
  ${LIBTHEORA_DIR}/lib/x86_vc/mmxfrag.c
  ${LIBTHEORA_DIR}/lib/x86_vc/mmxfrag.h
  ${LIBTHEORA_DIR}/lib/x86_vc/mmxidct.c
  ${LIBTHEORA_DIR}/lib/x86_vc/mmxloop.h
  ${LIBTHEORA_DIR}/lib/x86_vc/mmxstate.c
  ${LIBTHEORA_DIR}/lib/x86_vc/x86enc.c
  ${LIBTHEORA_DIR}/lib/x86_vc/x86enc.h
  ${LIBTHEORA_DIR}/lib/x86_vc/x86int.h
  ${LIBTHEORA_DIR}/lib/x86_vc/x86state.c

  ${LIBTHEORA_DIR}/lib/analyze.c
  ${LIBTHEORA_DIR}/lib/apiwrapper.c
  ${LIBTHEORA_DIR}/lib/bitpack.c
  ${LIBTHEORA_DIR}/lib/cpu.c
  ${LIBTHEORA_DIR}/lib/decapiwrapper.c
  ${LIBTHEORA_DIR}/lib/decinfo.c
  ${LIBTHEORA_DIR}/lib/decode.c
  ${LIBTHEORA_DIR}/lib/dequant.c
  ${LIBTHEORA_DIR}/lib/encapiwrapper.c
  ${LIBTHEORA_DIR}/lib/encfrag.c
  ${LIBTHEORA_DIR}/lib/encinfo.c
  ${LIBTHEORA_DIR}/lib/encode.c
  #${LIBTHEORA_DIR}/lib/encoder_disabled.c
  ${LIBTHEORA_DIR}/lib/enquant.c
  ${LIBTHEORA_DIR}/lib/fdct.c
  ${LIBTHEORA_DIR}/lib/fragment.c
  ${LIBTHEORA_DIR}/lib/huffdec.c
  ${LIBTHEORA_DIR}/lib/huffenc.c
  ${LIBTHEORA_DIR}/lib/idct.c
  ${LIBTHEORA_DIR}/lib/info.c
  ${LIBTHEORA_DIR}/lib/internal.c
  ${LIBTHEORA_DIR}/lib/mathops.c
  ${LIBTHEORA_DIR}/lib/mcenc.c
  ${LIBTHEORA_DIR}/lib/quant.c
  ${LIBTHEORA_DIR}/lib/rate.c
  ${LIBTHEORA_DIR}/lib/state.c
  ${LIBTHEORA_DIR}/lib/tokenize.c

  ${LIBTHEORA_DIR}/lib/apiwrapper.h
  ${LIBTHEORA_DIR}/lib/bitpack.h
  ${LIBTHEORA_DIR}/lib/cpu.h
  ${LIBTHEORA_DIR}/lib/dct.h
  ${LIBTHEORA_DIR}/lib/decint.h
  ${LIBTHEORA_DIR}/lib/dequant.h
  ${LIBTHEORA_DIR}/lib/encint.h
  ${LIBTHEORA_DIR}/lib/enquant.h
  ${LIBTHEORA_DIR}/lib/huffdec.h
  ${LIBTHEORA_DIR}/lib/huffenc.h
  ${LIBTHEORA_DIR}/lib/huffman.h
  #${LIBTHEORA_DIR}/lib/idct.h
  ${LIBTHEORA_DIR}/lib/internal.h
  ${LIBTHEORA_DIR}/lib/mathops.h
  ${LIBTHEORA_DIR}/lib/modedec.h
  ${LIBTHEORA_DIR}/lib/ocintrin.h
  ${LIBTHEORA_DIR}/lib/quant.h
)

target_compile_options(libtheora
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/permissive->
  $<$<CXX_COMPILER_ID:MSVC>:/wd4018>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4132>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4245>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4334>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4267>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4389>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4456>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4554>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4700>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4701>
  $<$<CXX_COMPILER_ID:Clang>:-Wno-parentheses>
  $<$<CXX_COMPILER_ID:Clang>:-Wno-shift-op-parentheses>
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