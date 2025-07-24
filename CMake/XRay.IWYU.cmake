set(
  USE_IWYU On
  CACHE BOOL
  "Use include-what-you-use to analyze #include structure."
)

if(USE_IWYU)
  find_program(IWYU_PROGRAM "include-what-you-use")
  if(IWYU_PROGRAM)
    message(STATUS "         IWYU: ${IWYU_PROGRAM}")
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE
      ${IWYU_PROGRAM}
      $<$<CXX_COMPILER_ID:MSVC>:--driver-mode=cl>
      -Xiwyu
      --cxx17ns
    )
    set(CMAKE_C_INCLUDE_WHAT_YOU_USE
      ${IWYU_PROGRAM}
      $<$<CXX_COMPILER_ID:MSVC>:--driver-mode=cl>
    )
  else()
    message(STATUS "         IWYU: Not Found")
  endif()
else()
  message(STATUS "         IWYU: Off")
endif()
