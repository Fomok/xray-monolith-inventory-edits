include_guard()

# Ensure VS will acknowledge our custom build types
set(CMAKE_CONFIGURATION_TYPES
  # Debug build - currently broken due to compile errors
  Debug
  # Pseudo-debug build
  Verified
  # Release build with profiling
  Profiled
  # Release build
  Release
  # Release build with PDB debug info
  RelWithDebInfo
)
