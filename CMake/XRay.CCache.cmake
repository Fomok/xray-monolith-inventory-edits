set(
  USE_CCACHE On
  CACHE BOOL
  "Use CCache to accelerate partial rebuilds."
)

if(USE_CCACHE)
  set(CCACHE_NAMESPACE anomaly)
  set(CCACHE_SLOPPINESS pch_defines,time_macros,include_file_ctime,include_file_mtime)

  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
    message(STATUS "       CCache: ${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER
      ${CCACHE_PROGRAM}
      namespace=${CCACHE_NAMESPACE}
      sloppiness=${CCACHE_SLOPPINESS}
    )
    set(CMAKE_C_COMPILER_LAUNCHER
      ${CCACHE_PROGRAM}
      namespace=anomaly
      sloppiness=${CCACHE_SLOPPINESS}
    )
  else()
    message(STATUS "       CCache: Not Found")
  endif()
else()
  message(STATUS "       CCache: Off")
endif()
