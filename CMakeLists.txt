# Use minimum VS-supported CMake for broad IDE compatibility
cmake_minimum_required(VERSION 3.20)

# Define project
project(
  Anomaly
  VERSION 2025.6.20
  DESCRIPTION "X-Ray Monolith Edition for S.T.A.L.K.E.R. Anomaly"
  LANGUAGES CXX C
)

# Setup language standard and enforce it
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Default generator platform to x64,
# as CMake doesn't populate it in case of default fallback (ex. via the GUI)
if(NOT CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM x64)
endif()

# Extend import path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Bring utilities into scope
include(util)

# Configure IDE folders
include(folders)

# Configure exceptions
configure_exceptions()

# Configure compiler options
add_compile_options(
  /Gd                   # __cdecl calling convention
  /diagnostics:column   # Enable rich diagnostics
  /Gm-                  # Disable minimal rebuild
  /fp:precise           # Precise floating-point mode
  /MP                   # Enable multi-process compilation
  /WX-                  # Don't treat warnings as errors
  /FC                   # Use full paths in diagnostic messages
  /Zi                   # Store debug information in PDB
)


# Configure linker options
add_link_options(
  /NXCOMPAT             # Mark verified with data execution prevention
  /LARGEADDRESSAWARE    # Enable large-address awareness
  /ERRORREPORT:PROMPT   # Enable error reporting prompt
  /DYNAMICBASE:NO       # Don't use a dynamic base address
)

# Suppress deprecation errors
add_compile_definitions(_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS)

# Setup Debug / Release configuration
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  add_compile_options(
    /Oy-                # Don't omit frame pointers
    /W4                 # Warning level 4
  )

  add_link_options(
    /OPT:NOICF          # Disable COMDAT folding
    /INCREMENTAL        # Enable incremental linking
    /FIXED:NO           # Don't use a fixed base address
    /OPT:NOREF          # Don't eliminate unreferenced functions and data
  )
else()
  # Remove built-in function inlining setting
  string(REPLACE "/Ob2" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

  add_compile_options(
    /Ob3                # Aggressive function inlining
    /GT                 # Fiber-safe optimizations
    /Gy                 # Function-level linking
    /Oi                 # Intrinsic functions
    /GF                 # String pooling
    /Oy                 # Omit frame pointers
    /GL                 # Whole-program optimization
  )

  add_link_options(
    /OPT:ICF            # Enable COMDAT folding
    /INCREMENTAL:NO     # Disable incremental linking
    /LTCG               # Use link-time code generation
    /OPT:REF            # Eliminate unreferenced functions and data
  )
endif()

# Setup profiler
set(PROFILER_NONE 0)
set(PROFILER_OPTICK 1)

if(${XRAY_PROFILER} STREQUAL "None")
  add_compile_definitions(
    XRCORE_PROFILER=${PROFILER_NONE}
  )
elseif(${XRAY_PROFILER} STREQUAL "Optick")
  add_compile_definitions(
    XRCORE_PROFILER=${PROFILER_OPTICK}
  )
endif()

# Setup gamedata
add_subdirectory(gamedata)

# Setup library targets
add_subdirectory(sdk)
add_subdirectory(src)

# Setup executable targets
include(targets)
