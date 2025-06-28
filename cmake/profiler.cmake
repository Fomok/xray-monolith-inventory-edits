include_guard()

# Profiler options
set(PROFILER_NONE 0)
set(PROFILER_OPTICK 1)

# Apply compile definition
if("${XRAY_PROFILER}" STREQUAL "None")
  add_compile_definitions(
    XRCORE_PROFILER=${PROFILER_NONE}
  )
elseif("${XRAY_PROFILER}" STREQUAL "Optick")
  add_compile_definitions(
    XRCORE_PROFILER=${PROFILER_OPTICK}
  )
endif()