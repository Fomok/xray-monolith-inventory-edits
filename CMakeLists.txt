# Use minimum VS-supported CMake for broad IDE compatibility
cmake_minimum_required(VERSION 3.20)

message(STATUS "CMake")
message(STATUS "      Version: ${CMAKE_VERSION}")
message(STATUS "    Generator: ${CMAKE_GENERATOR}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

# Configure Win32-specific before-project settings
if (WIN32)
  include(XRay.Win32)
endif()

include(Today)

include(XRay.Git)

# Define project
project(
  Anomaly
  VERSION ${TODAY_DOTS}
  DESCRIPTION "X-Ray Monolith Edition for S.T.A.L.K.E.R. Anomaly"
  LANGUAGES CXX C
)

message(STATUS)
message(STATUS "S.T.A.L.K.E.R. Anomaly")
message(STATUS "      Version: ${TODAY_DOTS}")

add_custom_target(CMake)
target_sources(CMake
  PRIVATE
  CMake/Today.cmake
  CMake/XRay.AVX.cmake
  CMake/XRay.cmake
  CMake/XRay.Compiler.cmake
  CMake/XRay.Compiler.Definitions.cmake
  CMake/XRay.Compiler.MSVC.cmake
  CMake/XRay.Compiler.Clang.cmake
  CMake/XRay.Compiler.Unsupported.cmake
  CMake/XRay.Configs.cmake
  CMake/XRay.Folders.cmake
  CMake/XRay.Git.cmake
  CMake/XRay.Linker.cmake
  CMake/XRay.Modules.cmake
  CMake/XRay.Paths.cmake
  CMake/XRay.Platform.cmake
  CMake/XRay.Win32.cmake
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