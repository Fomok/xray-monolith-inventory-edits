add_library(CxImage STATIC)

target_folder(CxImage ${FOLDER_EXTERNAL})

set(CXIMAGE_DIR ../CxImage-git/CxImage)

target_sources_sandboxed(CxImage
  DIRECTORY CxImage

  STRIP
  ${CXIMAGE_DIR}/

  SOURCES
  ximacfg.h

  ${CXIMAGE_DIR}/ximaenc.cpp
  ${CXIMAGE_DIR}/ximaexif.cpp
  ${CXIMAGE_DIR}/xImage.cpp
  ${CXIMAGE_DIR}/ximainfo.cpp
  ${CXIMAGE_DIR}/ximaint.cpp
  ${CXIMAGE_DIR}/ximajpg.cpp
  ${CXIMAGE_DIR}/ximalpha.cpp
  ${CXIMAGE_DIR}/ximatran.cpp
  ${CXIMAGE_DIR}/ximalyr.cpp
  ${CXIMAGE_DIR}/ximapal.cpp
  ${CXIMAGE_DIR}/ximasel.cpp
  ${CXIMAGE_DIR}/ximath.cpp
  ${CXIMAGE_DIR}/xmemfile.cpp

  ${CXIMAGE_DIR}/xfile.h
  ${CXIMAGE_DIR}/ximadef.h
  ${CXIMAGE_DIR}/ximage.h
  ${CXIMAGE_DIR}/ximaiter.h
  ${CXIMAGE_DIR}/ximajpg.h
  ${CXIMAGE_DIR}/ximath.h
  ${CXIMAGE_DIR}/xiofile.h
  ${CXIMAGE_DIR}/xmemfile.h
)

target_include_directories(CxImage
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/CxImage
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../cximage-git/jasper/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../cximage-git/zlib
)

target_compile_options(CxImage
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4267>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4324>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4456>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4457>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4458>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4611>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4702>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
  $<$<CXX_COMPILER_ID:MSVC>:/wd5033>
  $<$<CXX_COMPILER_ID:Clang>:-Wno-switch>
)

target_compile_definitions(CxImage
  PUBLIC
  $<$<CXX_COMPILER_ID:MSVC>:JAS_WIN_MSVC_BUILD>
  _CRT_SECURE_NO_WARNINGS
)

target_link_libraries(CxImage
  PRIVATE
  libjpeg
)
