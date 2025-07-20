# Vorbis
add_library(libvorbis STATIC)

target_folder(libvorbis ${FOLDER_EXTERNAL})

set(LIBVORBIS_DIR ../libvorbis-git)

target_sources_sandboxed(libvorbis
  DIRECTORY vorbis

  STRIP
  ${LIBVORBIS_DIR}/include/vorbis/
  ${LIBVORBIS_DIR}/lib/

  SOURCES
  ${LIBVORBIS_DIR}/include/vorbis/codec.h
  ${LIBVORBIS_DIR}/include/vorbis/vorbisenc.h

  ${LIBVORBIS_DIR}/lib/modes/setup_44p51.h
  ${LIBVORBIS_DIR}/lib/modes/residue_44p51.h

  ${LIBVORBIS_DIR}/lib/books/coupled/res_books_51.h

  ${LIBVORBIS_DIR}/lib/analysis.c
  ${LIBVORBIS_DIR}/lib/bitrate.c
  ${LIBVORBIS_DIR}/lib/block.c
  ${LIBVORBIS_DIR}/lib/codebook.c
  ${LIBVORBIS_DIR}/lib/envelope.c
  ${LIBVORBIS_DIR}/lib/floor0.c
  ${LIBVORBIS_DIR}/lib/floor1.c
  ${LIBVORBIS_DIR}/lib/info.c
  ${LIBVORBIS_DIR}/lib/lookup.c
  ${LIBVORBIS_DIR}/lib/lpc.c
  ${LIBVORBIS_DIR}/lib/lsp.c
  ${LIBVORBIS_DIR}/lib/mapping0.c
  ${LIBVORBIS_DIR}/lib/mdct.c
  ${LIBVORBIS_DIR}/lib/psy.c
  ${LIBVORBIS_DIR}/lib/registry.c
  ${LIBVORBIS_DIR}/lib/res0.c
  ${LIBVORBIS_DIR}/lib/sharedbook.c
  ${LIBVORBIS_DIR}/lib/smallft.c
  ${LIBVORBIS_DIR}/lib/synthesis.c
  ${LIBVORBIS_DIR}/lib/vorbisenc.c
  ${LIBVORBIS_DIR}/lib/window.c

  ${LIBVORBIS_DIR}/lib/backends.h
  ${LIBVORBIS_DIR}/lib/bitrate.h
  ${LIBVORBIS_DIR}/lib/books/coupled/res_books_stereo.h
  ${LIBVORBIS_DIR}/lib/books/floor/floor_books.h
  ${LIBVORBIS_DIR}/lib/books/uncoupled/res_books_uncoupled.h
  ${LIBVORBIS_DIR}/lib/codebook.h
  ${LIBVORBIS_DIR}/lib/codec_internal.h
  ${LIBVORBIS_DIR}/lib/envelope.h
  ${LIBVORBIS_DIR}/lib/highlevel.h
  ${LIBVORBIS_DIR}/lib/lookup.h
  ${LIBVORBIS_DIR}/lib/lookup_data.h
  ${LIBVORBIS_DIR}/lib/lpc.h
  ${LIBVORBIS_DIR}/lib/lsp.h
  ${LIBVORBIS_DIR}/lib/masking.h
  ${LIBVORBIS_DIR}/lib/mdct.h
  ${LIBVORBIS_DIR}/lib/misc.h
  ${LIBVORBIS_DIR}/lib/modes/floor_all.h
  ${LIBVORBIS_DIR}/lib/modes/psych_11.h
  ${LIBVORBIS_DIR}/lib/modes/psych_16.h
  ${LIBVORBIS_DIR}/lib/modes/psych_44.h
  ${LIBVORBIS_DIR}/lib/modes/psych_8.h
  ${LIBVORBIS_DIR}/lib/modes/residue_16.h
  ${LIBVORBIS_DIR}/lib/modes/residue_44.h
  ${LIBVORBIS_DIR}/lib/modes/residue_44u.h
  ${LIBVORBIS_DIR}/lib/modes/residue_8.h
  ${LIBVORBIS_DIR}/lib/modes/setup_11.h
  ${LIBVORBIS_DIR}/lib/modes/setup_16.h
  ${LIBVORBIS_DIR}/lib/modes/setup_22.h
  ${LIBVORBIS_DIR}/lib/modes/setup_32.h
  ${LIBVORBIS_DIR}/lib/modes/setup_44.h
  ${LIBVORBIS_DIR}/lib/modes/setup_44u.h
  ${LIBVORBIS_DIR}/lib/modes/setup_8.h
  ${LIBVORBIS_DIR}/lib/modes/setup_X.h
  ${LIBVORBIS_DIR}/lib/os.h
  ${LIBVORBIS_DIR}/lib/psy.h
  ${LIBVORBIS_DIR}/lib/registry.h
  ${LIBVORBIS_DIR}/lib/scales.h
  ${LIBVORBIS_DIR}/lib/smallft.h
  ${LIBVORBIS_DIR}/lib/window.h
)

target_include_directories(libvorbis
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(libvorbis
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/permissive->
  $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4267>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4305>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4456>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4457>
  $<$<CXX_COMPILER_ID:Clang>:-Wno-microsoft-include>
)

target_compile_definitions(libvorbis
  PRIVATE
  ANALYSIS
  _CRT_SECURE_NO_WARNINGS
)

target_link_libraries(libvorbis
  PRIVATE
  libogg
)

# Vorbis File
add_library(libvorbisfile STATIC)

target_folder(libvorbisfile ${FOLDER_EXTERNAL})

target_sources_sandboxed(libvorbisfile
  DIRECTORY vorbis

  STRIP
  ${LIBVORBIS_DIR}/lib/
  ${LIBVORBIS_DIR}/include/vorbis/

  SOURCES
  ${LIBVORBIS_DIR}/lib/vorbisfile.c
  ${LIBVORBIS_DIR}/include/vorbis/vorbisfile.h
)

target_include_directories(libvorbisfile
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(libvorbisfile
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/permissive->
  $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4267>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4456>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4457>
)

target_compile_definitions(libvorbisfile
  PRIVATE
  _CRT_SECURE_NO_WARNINGS
)

target_link_libraries(libvorbisfile
  PRIVATE
  libogg
)