add_library(XRay.Core.Profiling INTERFACE)

set_property(
  TARGET XRay.Core.Profiling
  PROPERTY FOLDER
  ${FOLDER_XRAY_CORE}
)

target_sources(XRay.Core.Profiling
  INTERFACE
  profiler.h
)
