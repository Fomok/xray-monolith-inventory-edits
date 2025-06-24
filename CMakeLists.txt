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

# Setup gamedata
add_subdirectory(gamedata)

# Setup library targets
add_subdirectory(sdk)
add_subdirectory(src)

# Setup executable targets
include(targets)

