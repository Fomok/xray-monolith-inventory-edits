add_library(libjpeg STATIC)

target_folder(libjpeg ${FOLDER_EXTERNAL})

set(LIBJPEG_DIR ../libjpeg-git)

target_sources_sandboxed(libjpeg
  DIRECTORY libjpeg
  
  STRIP
  ${LIBJPEG_DIR}/

  SOURCES
  jconfig.h

  ${LIBJPEG_DIR}/ansi2knr.c
  ${LIBJPEG_DIR}/cdjpeg.c
  ${LIBJPEG_DIR}/cjpeg.c
  ${LIBJPEG_DIR}/ckconfig.c
  ${LIBJPEG_DIR}/djpeg.c
  ${LIBJPEG_DIR}/jcapimin.c
  ${LIBJPEG_DIR}/jcapistd.c
  ${LIBJPEG_DIR}/jccoefct.c
  ${LIBJPEG_DIR}/jccolor.c
  ${LIBJPEG_DIR}/jcdctmgr.c
  ${LIBJPEG_DIR}/jchuff.c
  ${LIBJPEG_DIR}/jcinit.c
  ${LIBJPEG_DIR}/jcmainct.c
  ${LIBJPEG_DIR}/jcmarker.c
  ${LIBJPEG_DIR}/jcmaster.c
  ${LIBJPEG_DIR}/jcomapi.c
  ${LIBJPEG_DIR}/jcparam.c
  ${LIBJPEG_DIR}/jcphuff.c
  ${LIBJPEG_DIR}/jcprepct.c
  ${LIBJPEG_DIR}/jcsample.c
  ${LIBJPEG_DIR}/jctrans.c
  ${LIBJPEG_DIR}/jdapimin.c
  ${LIBJPEG_DIR}/jdapistd.c
  ${LIBJPEG_DIR}/jdatadst.c
  ${LIBJPEG_DIR}/jdatasrc.c
  ${LIBJPEG_DIR}/jdcoefct.c
  ${LIBJPEG_DIR}/jdcolor.c
  ${LIBJPEG_DIR}/jddctmgr.c
  ${LIBJPEG_DIR}/jdhuff.c
  ${LIBJPEG_DIR}/jdinput.c
  ${LIBJPEG_DIR}/jdmainct.c
  ${LIBJPEG_DIR}/jdmarker.c
  ${LIBJPEG_DIR}/jdmaster.c
  ${LIBJPEG_DIR}/jdmerge.c
  ${LIBJPEG_DIR}/jdphuff.c
  ${LIBJPEG_DIR}/jdpostct.c
  ${LIBJPEG_DIR}/jdsample.c
  ${LIBJPEG_DIR}/jdtrans.c
  ${LIBJPEG_DIR}/jerror.c
  ${LIBJPEG_DIR}/jfdctflt.c
  ${LIBJPEG_DIR}/jfdctfst.c
  ${LIBJPEG_DIR}/jfdctint.c
  ${LIBJPEG_DIR}/jidctflt.c
  ${LIBJPEG_DIR}/jidctfst.c
  ${LIBJPEG_DIR}/jidctint.c
  ${LIBJPEG_DIR}/jidctred.c
  ${LIBJPEG_DIR}/jmemansi.c
  ${LIBJPEG_DIR}/jmemmgr.c
  ${LIBJPEG_DIR}/jmemname.c
  ${LIBJPEG_DIR}/jmemnobs.c
  ${LIBJPEG_DIR}/jpegtran.c
  ${LIBJPEG_DIR}/jquant1.c
  ${LIBJPEG_DIR}/jquant2.c
  ${LIBJPEG_DIR}/jutils.c
  ${LIBJPEG_DIR}/rdbmp.c
  ${LIBJPEG_DIR}/rdcolmap.c
  ${LIBJPEG_DIR}/rdgif.c
  ${LIBJPEG_DIR}/rdjpgcom.c
  ${LIBJPEG_DIR}/rdppm.c
  ${LIBJPEG_DIR}/rdrle.c
  ${LIBJPEG_DIR}/rdswitch.c
  ${LIBJPEG_DIR}/rdtarga.c
  ${LIBJPEG_DIR}/transupp.c
  ${LIBJPEG_DIR}/wrbmp.c
  ${LIBJPEG_DIR}/wrgif.c
  ${LIBJPEG_DIR}/wrjpgcom.c
  ${LIBJPEG_DIR}/wrppm.c
  ${LIBJPEG_DIR}/wrrle.c
  ${LIBJPEG_DIR}/wrtarga.c

  ${LIBJPEG_DIR}/cderror.h
  ${LIBJPEG_DIR}/cdjpeg.h
  ${LIBJPEG_DIR}/jchuff.h
  ${LIBJPEG_DIR}/jdct.h
  ${LIBJPEG_DIR}/jdhuff.h
  ${LIBJPEG_DIR}/jerror.h
  ${LIBJPEG_DIR}/jinclude.h
  ${LIBJPEG_DIR}/jmemsys.h
  ${LIBJPEG_DIR}/jmorecfg.h
  ${LIBJPEG_DIR}/jpegint.h
  ${LIBJPEG_DIR}/jpeglib.h
  ${LIBJPEG_DIR}/jversion.h
  ${LIBJPEG_DIR}/transupp.h
)

target_include_directories(libjpeg
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/libjpeg
)

target_compile_options(libjpeg
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/wd4005>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4013>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4127>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4131>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4267>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4456>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4701>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4702>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4703>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
  $<$<CXX_COMPILER_ID:Clang>:-Wno-deprecated-declarations>
  $<$<CXX_COMPILER_ID:Clang>:-Wno-deprecated-non-prototype>
  $<$<CXX_COMPILER_ID:Clang>:-Wno-incompatible-library-redeclaration>
  $<$<CXX_COMPILER_ID:Clang>:-Wno-macro-redefined>
)

target_compile_definitions(libjpeg
  PRIVATE
  _CRT_SECURE_NO_WARNINGS
)
