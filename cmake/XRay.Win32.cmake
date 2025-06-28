include_guard()

# Setup runtime library
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

# Set VS debug libraries setting
set(CMAKE_VS_USE_DEBUG_LIBRARIES "$<CONFIG:Debug,Verified>")
