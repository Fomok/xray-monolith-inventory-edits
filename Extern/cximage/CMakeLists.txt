add_library(CxImage STATIC)

target_folder(CxImage ${FOLDER_EXTERNAL})

# Sandbox sources into the compilation directory to prevent inclusion of stdint.h, ximacfg.h
configure_file(../cximage-git/CxImage/ximaenc.cpp ximaenc.cpp COPYONLY)
configure_file(../cximage-git/CxImage/ximaexif.cpp ximaexif.cpp COPYONLY)
configure_file(../cximage-git/CxImage/xImage.cpp xImage.cpp COPYONLY)
configure_file(../cximage-git/CxImage/ximainfo.cpp ximainfo.cpp COPYONLY)
configure_file(../cximage-git/CxImage/ximaint.cpp ximaint.cpp COPYONLY)
configure_file(../cximage-git/CxImage/ximajpg.cpp ximajpg.cpp COPYONLY)
configure_file(../cximage-git/CxImage/ximalpha.cpp ximalpha.cpp COPYONLY)
configure_file(../cximage-git/CxImage/ximatran.cpp ximatran.cpp COPYONLY)
configure_file(../cximage-git/CxImage/ximalyr.cpp ximalyr.cpp COPYONLY)
configure_file(../cximage-git/CxImage/ximapal.cpp ximapal.cpp COPYONLY)
configure_file(../cximage-git/CxImage/ximasel.cpp ximasel.cpp COPYONLY)
configure_file(../cximage-git/CxImage/ximath.cpp ximath.cpp COPYONLY)
configure_file(../cximage-git/CxImage/xmemfile.cpp xmemfile.cpp COPYONLY)

configure_file(../cximage-git/CxImage/xfile.h xfile.h COPYONLY)
configure_file(../cximage-git/CxImage/ximadef.h ximadef.h COPYONLY)
configure_file(../cximage-git/CxImage/ximage.h ximage.h COPYONLY)
configure_file(../cximage-git/CxImage/ximaiter.h ximaiter.h COPYONLY)
configure_file(../cximage-git/CxImage/ximajpg.h ximajpg.h COPYONLY)
configure_file(../cximage-git/CxImage/ximath.h ximath.h COPYONLY)
configure_file(../cximage-git/CxImage/xiofile.h xiofile.h COPYONLY)
configure_file(../cximage-git/CxImage/xmemfile.h xmemfile.h COPYONLY)

target_sources(CxImage
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/ximaenc.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/ximaexif.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/xImage.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/ximainfo.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/ximaint.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/ximajpg.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/ximalpha.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/ximatran.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/ximalyr.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/ximapal.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/ximasel.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/ximath.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/xmemfile.cpp

  ${CMAKE_CURRENT_BINARY_DIR}/xfile.h
  ${CMAKE_CURRENT_BINARY_DIR}/ximadef.h
  ${CMAKE_CURRENT_BINARY_DIR}/ximage.h
  ${CMAKE_CURRENT_BINARY_DIR}/ximaiter.h
  ${CMAKE_CURRENT_BINARY_DIR}/ximajpg.h
  ${CMAKE_CURRENT_BINARY_DIR}/ximath.h
  ${CMAKE_CURRENT_BINARY_DIR}/xiofile.h
  ${CMAKE_CURRENT_BINARY_DIR}/xmemfile.h

  ximacfg.h
)

target_include_directories(CxImage
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
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
