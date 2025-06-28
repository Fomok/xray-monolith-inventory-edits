# Use minimum VS-supported CMake for broad IDE compatibility
cmake_minimum_required(VERSION 3.20)

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
set(CMAKE_VS_USE_DEBUG_LIBRARIES "$<CONFIG:Debug,Verified>")

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

# Extend import path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Ensure VS will acknowledge our custom build types
set(CMAKE_CONFIGURATION_TYPES
  # Debug build - currently broken due to compile errors
  Debug
  # Pseudo-debug build
  Verified
  # Release build with no debug info
  Release
  # Release build with PDB debug info
  RelWithDebInfo
)

# Use Debug flags for Verified builds
set(CMAKE_EXE_LINKER_FLAGS_VERIFIED ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
set(CMAKE_SHARED_LINKER_FLAGS_VERIFIED ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
set(CMAKE_STATIC_LINKER_FLAGS_VERIFIED ${CMAKE_STATIC_LINKER_FLAGS_DEBUG})

#set(CMAKE_CXX_FLAGS_VERIFIED ${CMAKE_CXX_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_VERIFIED "${CMAKE_CXX_FLAGS_DEBUG} /DNDEBUG")
string(
  REPLACE
  "/RTC1" ""
  CMAKE_CXX_FLAGS_VERIFIED "${CMAKE_CXX_FLAGS_VERIFIED}"
)

set(CMAKE_C_FLAGS_VERIFIED "${CMAKE_C_FLAGS_DEBUG} /DNDEBUG")
string(
  REPLACE
  "/RTC1" ""
  CMAKE_C_FLAGS_VERIFIED "${CMAKE_C_FLAGS_VERIFIED}"
)

# Default generator platform to x64,
# as CMake doesn't populate it in case of default fallback (ex. via the GUI)
if(NOT CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM x64)
endif()

# Bring utilities into scope
include(util)

# Configure IDE folders
include(folders)

# Remove built-in CMake flags
string(
  REPLACE
  "/Ob2" ""
  CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}"
)

string(
  REPLACE "/Ob1" ""
  CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
)

# Configure compiler options
add_compile_options(
  ## Shared config
  # Enable rich diagnostics
  /diagnostics:column
  # Disable minimal rebuild
  /Gm-
  # Precise floating-point mode
  /fp:precise
  # Enable multi-process compilation
  /MP
  # Don't treat warnings as errors
  /WX-
  # Use full paths in diagnostic messages
  /FC
  # Store debug information in PDB
  /Zi
  # Use multi-threaded DLL
  /MD

  ## Debug config
  # Don't omit frame pointers
  $<$<CONFIG:Debug>:/Oy->
  $<$<CONFIG:Verified>:/Oy->
  # Warning level 4
  $<$<CONFIG:Debug>:/W4>
  $<$<CONFIG:Verified>:/W4>
  # Enable security check
  $<$<CONFIG:Debug>:/GS>
  $<$<CONFIG:Verified>:/GS>

  ## Release configs
  # Aggressive function inlining
  $<$<CONFIG:Release>:/Ob3>
  $<$<CONFIG:RelWithDebInfo>:/Ob3>
  # Omit frame pointers
  $<$<CONFIG:Release>:/Oy>
  $<$<CONFIG:RelWithDebInfo>:/Oy>
  # Fiber-safe optimizations
  $<$<CONFIG:Release>:/GT>
  $<$<CONFIG:RelWithDebInfo>:/GT>
  # Function-level linking
  $<$<CONFIG:Release>:/Gy>
  $<$<CONFIG:RelWithDebInfo>:/Gy>
  # Intrinsic functions
  $<$<CONFIG:Release>:/Oi>
  $<$<CONFIG:RelWithDebInfo>:/Oi>
  # String pooling
  $<$<CONFIG:Release>:/GF>
  $<$<CONFIG:RelWithDebInfo>:/GF>
  # Whole-program optimization
  $<$<CONFIG:Release>:/GL>
  $<$<CONFIG:RelWithDebInfo>:/GL>
)

# Configure linker options
add_link_options(
  ## Shared config
  # Mark verified with data execution prevention
  /NXCOMPAT
  # Enable large-address awareness
  /LARGEADDRESSAWARE
  # Enable error reporting prompt
  /ERRORREPORT:PROMPT
  # Don't use a dynamic base address
  /DYNAMICBASE:NO

  ## Debug config
  # Disable COMDAT folding
  $<$<CONFIG:Debug>:/OPT:NOICF>
  $<$<CONFIG:Verified>:/OPT:NOICF>
  # Link incrementally
  $<$<CONFIG:Debug>:/INCREMENTAL>
  $<$<CONFIG:Verified>:/INCREMENTAL>
  # Don't eliminate unreferenced functions and data
  $<$<CONFIG:Debug>:/OPT:NOREF>
  $<$<CONFIG:Verified>:/OPT:NOREF>
  # Don't use a fixed address space
  $<$<CONFIG:Debug>:/FIXED:NO>
  $<$<CONFIG:Verified>:/FIXED:NO>

  ## Release / RelWithDebInfo config
  # Enable COMDAT folding
  $<$<CONFIG:Release>:/OPT:ICF>
  $<$<CONFIG:RelWithDebInfo>:/OPT:ICF>
  # Don't link incrementally
  $<$<CONFIG:Release>:/INCREMENTAL:NO>
  $<$<CONFIG:RelWithDebInfo>:/INCREMENTAL:NO>
  # Eliminate unreferenced functions and data
  $<$<CONFIG:Release>:/OPT:REF>
  $<$<CONFIG:RelWithDebInfo>:/OPT:REF>
  # Enable link-time code generation
  $<$<CONFIG:Release>:/LTCG>
  $<$<CONFIG:RelWithDebInfo>:/LTCG>
)

# Configure compile definitions
add_compile_definitions(
  # Suppress deprecation errors
  _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
  # Activate verification for Verified builds
  $<$<CONFIG:Verified>:USE_VERIFY_IN_RELEASE>
)

# Configure exceptions
configure_exceptions()

# Setup profiler
set(PROFILER_NONE 0)
set(PROFILER_OPTICK 1)

if("${XRAY_PROFILER}" STREQUAL "None")
  add_compile_definitions(
    XRCORE_PROFILER=${PROFILER_NONE}
  )
elseif("${XRAY_PROFILER}" STREQUAL "Optick")
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
