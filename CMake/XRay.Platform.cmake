# Default generator platform to x64,
# as CMake doesn't populate it in case of default fallback (ex. via the GUI)
if(NOT CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM x64)
endif()
message(STATUS "     Platform: ${CMAKE_SYSTEM_NAME} ${CMAKE_GENERATOR_PLATFORM}")