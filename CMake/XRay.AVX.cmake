if(${XRAY_AVX})
  add_compile_options(/arch:AVX)
endif()

message(STATUS "          AVX: ${XRAY_AVX}")