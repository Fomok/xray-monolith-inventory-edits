# Use minimum VS-supported CMake for broad IDE compatibility
cmake_minimum_required(VERSION 3.20)

# Given a CMake module name, add ./CMake to the module path and include it
macro(add_cmake_directory NAME)
  # Update module path
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

  # Hand off to CMake directory
  include(${NAME})
endmacro()

# Hand off to our XRay entrypoint
add_cmake_directory(XRay)