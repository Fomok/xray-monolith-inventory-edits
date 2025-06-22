cmake_minimum_required(VERSION 4.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Default to x64, as CMake doesn't populate it in case of default fallback (ex. via the GUI)
if(NOT CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM x64)
endif()

# Configure folders
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(FOLDER_XRAY "X-Ray")
set(FOLDER_3RD_PARTY "X-Ray/3rd Party")
set(FOLDER_LUA "X-Ray/Lua")
set(FOLDER_RENDER "X-Ray/Render")
set(FOLDER_SDK "X-Ray/SDK")
set(FOLDER_GAMEDATA "X-Ray/Gamedata")

include(util)
include(msvc)

project(Anomaly VERSION 2025.6.20
                DESCRIPTION "X-Ray Monolith Edition for S.T.A.L.K.E.R. Anomaly"
                LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable parallel building for the given target
macro(enable_parallel_build TARGET)
  if(MSVC)
    target_compile_options(${TARGET} PRIVATE "/MP")
  endif()
endmacro()

# Hand off to subdirectories
add_subdirectory("sdk")
add_subdirectory("src")
add_subdirectory("gamedata")
