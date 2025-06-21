# TODO:
# - [X] Compile all modules in isolation
# - [-] Compile and link via xrEngine
# - [ ] Strip redundant preprocessor definitions
# - [ ] Reintegrate existing DX / AVX build configurations and their settings
# - [ ] Enable / disable exceptions based on build type
# - [ ] Fix wonky StackWalker library
#   - Library and header are both stored inside xrGame instead of freestanding
# - [ ] Replace stdext::hash_map with std::unordered_map
#   - Allows removal of silence_stdext_hash_deprecation_warnings
# - [ ] Clean up relative include paths

cmake_minimum_required(VERSION 4.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Default to x64, as CMake doesn't populate it in case of default fallback (ex. via the GUI)
if(NOT CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM x64)
endif()

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
