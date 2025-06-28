# Use minimum VS-supported CMake for broad IDE compatibility
cmake_minimum_required(VERSION 3.20)

# Update module path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Configure Win32-specific before-project settings
if (WIN32)
  include(XRay.Win32)
endif()

# Define project
project(
  Anomaly
  VERSION 2025.6.20
  DESCRIPTION "X-Ray Monolith Edition for S.T.A.L.K.E.R. Anomaly"
  LANGUAGES CXX C
)

# Default generator platform to x64,
# as CMake doesn't populate it in case of default fallback (ex. via the GUI)
if(NOT CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM x64)
endif()

# Setup language standard and enforce it
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Bring utilities into scope
include(sources)
include(manifest)

# Setup build configurations
include(configs)

# Define paths
include(paths)

# Configure IDE folders
include(folders)

# Setup global compiler definitions
include(definitions)

# Configure toolchain-specific settings
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  include(XRay.MSVC)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  include(XRay.Clang)
else()
  error("Unsupported compiler")
endif()

# Configure profiler
include(profiler)

# Setup compressor
add_subdirectory(compressor)

# Setup gamedata
add_subdirectory(gamedata)

# Setup SDK
add_subdirectory(sdk)

# Setup engine sources
add_subdirectory(src)

# Setup executable targets
include(targets)
