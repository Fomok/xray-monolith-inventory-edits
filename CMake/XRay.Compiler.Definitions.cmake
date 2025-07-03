include_guard()

# Configure compile definitions
add_compile_definitions(
  # Suppress deprecation errors
  _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
)

# Activate interprocedural optimization for release builds
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_PROFILED On)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE On)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO On)