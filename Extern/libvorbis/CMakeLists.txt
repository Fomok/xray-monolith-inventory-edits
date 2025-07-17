# Vorbis
add_library(libvorbis STATIC)

target_folder(libvorbis ${FOLDER_EXTERNAL})

target_sources_sandboxed(libvorbis
  DIRECTORY vorbis
  SOURCES
  lib/analysis.c
  lib/bitrate.c
  lib/block.c
  lib/codebook.c
  lib/envelope.c
  lib/floor0.c
  lib/floor1.c
  lib/info.c
  lib/lookup.c
  lib/lpc.c
  lib/lsp.c
  lib/mapping0.c
  lib/mdct.c
  lib/psy.c
  lib/registry.c
  lib/res0.c
  lib/sharedbook.c
  lib/smallft.c
  lib/synthesis.c
  lib/vorbisenc.c
  lib/window.c

  codec.h
  vorbisenc.h
  lib/backends.h
  lib/bitrate.h
  lib/books/coupled/res_books_stereo.h
  lib/books/floor/floor_books.h
  lib/books/uncoupled/res_books_uncoupled.h
  lib/codebook.h
  lib/codec_internal.h
  lib/envelope.h
  lib/highlevel.h
  lib/lookup.h
  lib/lookup_data.h
  lib/lpc.h
  lib/lsp.h
  lib/masking.h
  lib/mdct.h
  lib/misc.h
  lib/modes/floor_all.h
  lib/modes/psych_11.h
  lib/modes/psych_16.h
  lib/modes/psych_44.h
  lib/modes/psych_8.h
  lib/modes/residue_16.h
  lib/modes/residue_44.h
  lib/modes/residue_44u.h
  lib/modes/residue_8.h
  lib/modes/setup_11.h
  lib/modes/setup_16.h
  lib/modes/setup_22.h
  lib/modes/setup_32.h
  lib/modes/setup_44.h
  lib/modes/setup_44u.h
  lib/modes/setup_8.h
  lib/modes/setup_X.h
  lib/os.h
  lib/psy.h
  lib/registry.h
  lib/scales.h
  lib/smallft.h
  lib/window.h
)

target_include_directories(libvorbis
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(libvorbis
  PRIVATE
  ANALYSIS
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
  SOURCES
  lib/vorbisfile.c
  vorbisfile.h
)

target_include_directories(libvorbisfile
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(libvorbisfile
  PRIVATE
  libogg
)