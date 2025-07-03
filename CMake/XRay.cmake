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

# Default generator platform to x64,
# as CMake doesn't populate it in case of default fallback (ex. via the GUI)
if(NOT CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM x64)
endif()

# Setup language standard and enforce it
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Setup build configurations
include(XRay.Configs)

# Configure toolchain-specific settings
if(${MSVC})
  include(XRay.MSVC)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  include(XRay.Clang)
else()
  message(FATAL_ERROR "Unsupported compiler")
endif()

# Setup compiler
include(XRay.Compiler)

# Setup linker
include(XRay.Linker)

# Define paths
include(XRay.Paths)

# Configure IDE folders
include(XRay.Folders)

# Setup global compiler definitions
include(XRay.Compiler.Definitions)

# Setup module definition machinery
include(XRay.Modules)

# Setup compressor
add_subdirectory(compressor)

# Setup gamedata
add_subdirectory(gamedata)

# Setup SDK
add_subdirectory(sdk)

# Setup engine sources
add_subdirectory(src)

# Set visual studio startup project
set_property(
  DIRECTORY ${CMAKE_SOURCE_DIR}
  PROPERTY VS_STARTUP_PROJECT
  AnomalyDX11AVX
)
