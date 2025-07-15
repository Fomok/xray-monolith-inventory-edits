add_library(lua-extensions STATIC)

target_folder(lua-extensions ${FOLDER_LUA})

target_sources(lua-extensions PRIVATE
  base.cpp
  lfs.c
  lmarshal.c
  script_additional_libs.cpp

  lfs.h
  lmarshal.h
  script_additional_libs.h

  luapanda.cpp
  luapanda.h
  
  luasocket/auxiliar.c
  luasocket/buffer.c
  luasocket/compat.c
  luasocket/except.c
  luasocket/inet.c
  luasocket/io.c
  luasocket/luasocket.c
  luasocket/mime.c
  luasocket/options.c
  luasocket/select.c
  luasocket/tcp.c
  luasocket/timeout.c
  luasocket/udp.c
  luasocket/wsocket.c
  luasocket/auxiliar.h

  luasocket/buffer.h
  luasocket/compat.h
  luasocket/except.h
  luasocket/inet.h
  luasocket/io.h
  luasocket/luasocket.h
  luasocket/mime.h
  luasocket/options.h
  luasocket/pierror.h
  luasocket/select.h
  luasocket/socket.h
  luasocket/tcp.h
  luasocket/timeout.h
  luasocket/udp.h
  luasocket/wsocket.h
)

target_include_directories(lua-extensions
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(lua-extensions
  PRIVATE
  LuaJIT
  XRay.Core
)
