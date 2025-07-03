# Use minimum VS-supported CMake for broad IDE compatibility
cmake_minimum_required(VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

# Configure Win32-specific before-project settings
if (WIN32)
  include(XRay.Win32)
endif()

include(Today)

# Define project
project(
  Anomaly
  VERSION ${TODAY_DOTS}
  DESCRIPTION "X-Ray Monolith Edition for S.T.A.L.K.E.R. Anomaly"
  LANGUAGES CXX C
)

# Hand off to our XRay entrypoint
include(XRay)

# Setup compressor
add_subdirectory(compressor)

# Setup gamedata
add_subdirectory(gamedata)

# Setup SDK
add_subdirectory(sdk)

# Setup engine sources
add_subdirectory(src)