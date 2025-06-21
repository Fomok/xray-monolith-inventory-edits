# Vorbis
add_library(libvorbis_static STATIC)

target_sources(libvorbis_static PRIVATE
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
)

target_include_directories(libvorbis_static
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/sdk/include"
)

target_compile_definitions(libvorbis_static
  PUBLIC
  ANALYSIS
  NDEBUG

  PRIVATE
  WIN32
  _WINDOWS
)

enable_parallel_build(libvorbis_static)

# Vorbis File
add_library(libvorbisfile_static STATIC)

target_sources(libvorbisfile_static PRIVATE
  lib/vorbisfile.c
)

target_include_directories(libvorbisfile_static
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/sdk/include"
)

target_compile_definitions(libvorbisfile_static
  PUBLIC
  NDEBUG

  PRIVATE
  WIN32
  _WINDOWS
)

enable_parallel_build(libvorbisfile_static)
