if(${XRAY_AVX})
  if(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
    add_compile_options(/arch:AVX)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    add_compile_options(-mavx)
  endif()
endif()

message(STATUS "          AVX: ${XRAY_AVX}")