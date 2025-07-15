add_library(crypto STATIC)

target_folder(crypto ${FOLDER_EXTERNAL})

target_sources(crypto PRIVATE
  crypto.cpp
  crypto.h

  crypto_api_def.h

  xr_dsa.cpp
  xr_sha.cpp
)

target_include_directories(crypto
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(crypto
  PRIVATE
  CRYPTO_BUILD
)

target_link_libraries(crypto
  PRIVATE
  OpenSSL
  XRay.Core
)