add_library(CxImage STATIC)

set_property(
  TARGET CxImage
  PROPERTY FOLDER
  ${FOLDER_3RD_PARTY}
)

target_sources(CxImage PRIVATE
  ximacfg.cpp
  ximaenc.cpp
  ximaexif.cpp
  xImage.cpp
  ximainfo.cpp
  ximaint.cpp
  ximajpg.cpp
  ximalpha.cpp
  ximalyr.cpp
  ximapal.cpp
  ximasel.cpp
  ximath.cpp
  xmemfile.cpp

  xfile.h
  ximacfg.h
  ximadef.h
  ximage.h
  ximaiter.h
  ximajpg.h
  ximath.h
  xiofile.h
  xmemfile.h
)

target_include_directories(CxImage
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/sdk/include"
)

target_compile_definitions(CxImage
  PUBLIC
  NDEBUG
  JAS_WIN_MSVC_BUILD
  CXIMAGE_BUILD

  PRIVATE
  WIN32
  _LIB
)

enable_parallel_build(CxImage)
