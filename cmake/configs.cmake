include_guard()

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

# Store compiler flag prefix in a variable
if(MSVC)
    set(CMAKE_FLAG_PREFIX "/")
else()
    set(CMAKE_FLAG_PREFIX "-")
endif()

# Setup verified configuration
set(CMAKE_CXX_FLAGS_VERIFIED "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_FLAG_PREFIX}DNDEBUG")
string(
  REPLACE
  "/RTC1" ""
  CMAKE_CXX_FLAGS_VERIFIED "${CMAKE_CXX_FLAGS_VERIFIED}"
)

set(CMAKE_C_FLAGS_VERIFIED "${CMAKE_C_FLAGS_DEBUG} ${CMAKE_FLAG_PREFIX}DNDEBUG")
string(
  REPLACE
  "/RTC1" ""
  CMAKE_C_FLAGS_VERIFIED "${CMAKE_C_FLAGS_VERIFIED}"
)

# Use Debug flags for Verified builds
set(CMAKE_EXE_LINKER_FLAGS_VERIFIED "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
set(CMAKE_SHARED_LINKER_FLAGS_VERIFIED "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
set(CMAKE_STATIC_LINKER_FLAGS_VERIFIED "${CMAKE_STATIC_LINKER_FLAGS_DEBUG}")
