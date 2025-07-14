add_library(CxImage STATIC)

target_folder(CxImage ${FOLDER_EXTERNAL})

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
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MSVC)
  target_compile_definitions(CxImage
    PUBLIC
    JAS_WIN_MSVC_BUILD
  )
endif()

target_compile_definitions(CxImage
  PRIVATE
  CXIMAGE_BUILD
)

target_link_libraries(CxImage
  PRIVATE
  jpeg
)
