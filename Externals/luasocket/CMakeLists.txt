add_library(luasocket STATIC)

target_folder(luasocket ${FOLDER_LUA_EXTENSIONS})

set(LUASOCKET_DIR ../luasocket-git)

target_sources_sandboxed(luasocket
  DIRECTORY luasocket
  STRIP
  ${LUASOCKET_DIR}/src/
  SOURCES
  ${LUASOCKET_DIR}/src/auxiliar.c
  ${LUASOCKET_DIR}/src/buffer.c
  ${LUASOCKET_DIR}/src/compat.c
  ${LUASOCKET_DIR}/src/except.c
  ${LUASOCKET_DIR}/src/inet.c
  ${LUASOCKET_DIR}/src/io.c
  ${LUASOCKET_DIR}/src/luasocket.c
  ${LUASOCKET_DIR}/src/mime.c
  ${LUASOCKET_DIR}/src/options.c
  ${LUASOCKET_DIR}/src/select.c
  ${LUASOCKET_DIR}/src/tcp.c
  ${LUASOCKET_DIR}/src/timeout.c
  ${LUASOCKET_DIR}/src/udp.c
  ${LUASOCKET_DIR}/src/wsocket.c
  ${LUASOCKET_DIR}/src/auxiliar.h

  ${LUASOCKET_DIR}/src/buffer.h
  ${LUASOCKET_DIR}/src/compat.h
  ${LUASOCKET_DIR}/src/except.h
  ${LUASOCKET_DIR}/src/inet.h
  ${LUASOCKET_DIR}/src/io.h
  ${LUASOCKET_DIR}/src/luasocket.h
  ${LUASOCKET_DIR}/src/mime.h
  ${LUASOCKET_DIR}/src/options.h
  ${LUASOCKET_DIR}/src/pierror.h
  ${LUASOCKET_DIR}/src/select.h
  ${LUASOCKET_DIR}/src/socket.h
  ${LUASOCKET_DIR}/src/tcp.h
  ${LUASOCKET_DIR}/src/timeout.h
  ${LUASOCKET_DIR}/src/udp.h
  ${LUASOCKET_DIR}/src/wsocket.h
)

target_include_directories(luasocket
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(luasocket
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/permissive->
  $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
)

target_compile_definitions(luasocket
  PRIVATE
  _WINSOCK_DEPRECATED_NO_WARNINGS
  _CRT_SECURE_NO_WARNINGS
)

target_link_libraries(luasocket
  PRIVATE
  LuaJIT
)
