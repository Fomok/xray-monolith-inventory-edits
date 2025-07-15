add_library(DxErr STATIC)

target_folder(DxErr ${FOLDER_EXTERNAL})

set_target_properties(DxErr PROPERTIES OUTPUT_NAME DxErr_x64_ar)

# Patch dxerr.cpp with Clang / GCC -compliant macro stringify and concat
file(READ ../DxErr-git/DxErr/src/dxerr.cpp DXERR_CPP)
string(REPLACE "L#hrchk" "L###hrchk" DXERR_CPP "${DXERR_CPP}")
string(REPLACE "L##__VA_ARGS__" "L###__VA_ARGS__" DXERR_CPP "${DXERR_CPP}")
file(GENERATE OUTPUT dxerr.cpp CONTENT "${DXERR_CPP}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS dxerr.cpp)

target_sources(DxErr
  PRIVATE
  ../DxErr-git/DxErr/src/dxerr.h
  ${CMAKE_CURRENT_BINARY_DIR}/dxerr.cpp
)

target_include_directories(DxErr
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../DxErr-git/DxErr/src
)
