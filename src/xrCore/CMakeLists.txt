macro(add_core_static NAME)
  add_library(${NAME} STATIC)

  target_sources(${NAME}
    PRIVATE
    ${ARGN}
  )

  target_include_directories(${NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  )

  target_precompile_headers(${NAME}
    PRIVATE
    stdafx.h
  )

  target_compile_definitions(${NAME}
    PRIVATE
    PURE_ALLOC
    XRCORE_EXPORTS
    PORTABLE_BUGSLAYERUTIL
  )

  target_link_libraries(${NAME}
    PRIVATE
    DXERR
    optick
    StackWalker
    winmm
    XRay.Render.API
    XRay.Collision
  )

  target_link_libraries(XRay.Core PRIVATE ${NAME})
endmacro()

macro(add_core_interface NAME)
  add_library(${NAME} INTERFACE)

  target_sources(${NAME}
    PRIVATE
    ${ARGN}
  )

  target_include_directories(${NAME}
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
  )

  target_link_libraries(XRay.Core PRIVATE ${NAME})
endmacro()

add_library(XRay.Core STATIC)

target_folder(XRay.Core ${FOLDER_XRAY_CORE})

target_sources(XRay.Core
  PRIVATE
  FTimer.cpp
  xrCore.cpp
  
  FTimer.h
  stdafx.h
  xrCore.h
  xrCore_platform.h
  
  ../build_config_defines.h
  ChooseTypes.H
  client_id.h
  robin_hood.h

  resource.h
  xrCore.rc
)

target_include_directories(XRay.Core
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.Collision
)

include(cmake/XRay.Core.Compression.LZ.cmake)
include(cmake/XRay.Core.Compression.LZO.cmake)
include(cmake/XRay.Core.Compression.PPMD.cmake)
include(cmake/XRay.Core.Compression.RT.cmake)
include(cmake/XRay.Core.Debug.cmake)
include(cmake/XRay.Core.Debug.BlackBox.cmake)
include(cmake/XRay.Core.FS.cmake)
include(cmake/XRay.Core.IntrusivePointer.cmake)
include(cmake/XRay.Core.Math.cmake)
include(cmake/XRay.Core.Memory.Manager.cmake)
include(cmake/XRay.Core.Memory.Monitor.cmake)
include(cmake/XRay.Core.Memory.Shared.cmake)
include(cmake/XRay.Core.OS.cmake)
include(cmake/XRay.Core.Profiling.cmake)
include(cmake/XRay.Core.Threading.cmake)
