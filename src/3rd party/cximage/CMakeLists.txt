add_library(CxImage STATIC)

target_sources(CxImage PRIVATE
  xfile.h
  ximacfg.cpp
  ximacfg.h
  ximadef.h
  ximaenc.cpp
  ximaexif.cpp
  ximage.cpp
  ximage.h
  ximainfo.cpp
  ximaint.cpp
  ximaiter.h
  ximajpg.cpp
  ximajpg.h
  ximalpha.cpp
  ximalyr.cpp
  ximapal.cpp
  ximasel.cpp
  ximath.cpp
  ximath.h
  xiofile.h
  xmemfile.cpp
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
