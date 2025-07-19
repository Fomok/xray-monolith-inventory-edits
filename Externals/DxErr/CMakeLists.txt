add_library(DxErr STATIC)

target_folder(DxErr ${FOLDER_EXTERNAL})

set_target_properties(DxErr PROPERTIES OUTPUT_NAME DxErr_x64_ar)

set(DXERR_DIR ../DxErr-git/DxErr/src)

# Patch dxerr.cpp with Clang / GCC -compliant macro stringify and concat
file(READ ${DXERR_DIR}/DxErr.cpp DXERR_CPP)
string(REPLACE "L#hrchk" "L###hrchk" DXERR_CPP "${DXERR_CPP}")
string(REPLACE "L##__VA_ARGS__" "L###__VA_ARGS__" DXERR_CPP "${DXERR_CPP}")
file(GENERATE OUTPUT DxErr/DxErr.cpp CONTENT "${DXERR_CPP}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS DxErr.cpp)
target_sources(DxErr PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/DxErr/DxErr.cpp)

target_sources_sandboxed(DxErr
  DIRECTORY DxErr

  STRIP
  ${DXERR_DIR}/

  SOURCES
  ${DXERR_DIR}/DxErr.h
  ${DXERR_DIR}/DxGetErrorString.inl
  ${DXERR_DIR}/DxGetErrorDescription.inl
  ${DXERR_DIR}/DxTrace.inl
)

target_include_directories(DxErr
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/DxErr
)

target_compile_options(DxErr
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/wd4313>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4474>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4477>
)
