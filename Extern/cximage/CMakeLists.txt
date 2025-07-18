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

if(MSVC)
  target_compile_definitions(CxImage
    PUBLIC
    JAS_WIN_MSVC_BUILD
  )
endif()

target_link_libraries(CxImage
  PRIVATE
  libjpeg
)
