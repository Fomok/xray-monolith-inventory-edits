add_library(XRay.Core.Profiling INTERFACE)

target_folder(XRay.Core.Profiling ${FOLDER_XRAY_CORE})

target_sources(XRay.Core.Profiling
  INTERFACE
  profiler.h
)
