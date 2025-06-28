# Given a target, name, scope, and list of files,
# add the files as sources and group them with the given name
# From OpenXRay `cmake/utils.cmake`
function(target_sources_grouped)
    cmake_parse_arguments(
        PARSED_ARGS
        ""
        "TARGET;NAME;SCOPE"
        "FILES"
        ${ARGN}
    )

    if(NOT PARSED_ARGS_TARGET)
        message(FATAL_ERROR "You must provide a target name")
    endif()

    if(NOT PARSED_ARGS_NAME)
        message(FATAL_ERROR "You must provide a source group name")
    endif()

    if(NOT PARSED_ARGS_SCOPE)
        set(PARSED_ARGS_SCOPE PRIVATE)
    endif()

    target_sources(${PARSED_ARGS_TARGET} ${PARSED_ARGS_SCOPE} ${PARSED_ARGS_FILES})

    source_group(${PARSED_ARGS_NAME} FILES ${PARSED_ARGS_FILES})
endfunction()

# Remove global exception flags and replace them with debug-only ones
macro(configure_exceptions)
  if(MSVC)
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /EHsc")
  else()
    add_compile_options(
      $<$<NOT:$<CONFIG:Debug>>:-fno-exceptions>
    )
  endif()
endmacro()

# Enable AVX for the given target
macro(enable_avx TARGET)
  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /arch:AVX)
  endif()
endmacro()

## Manifest tool abstraction
macro(add_executable_manifest target manifest)
  if (MSVC)
    add_custom_command(
        TARGET ${target}
        POST_BUILD
      COMMAND mt -manifest ${manifest} -outputresource:$<TARGET_FILE:${target}>
        COMMENT "Adding manifest..." 
    )
  endif()
endmacro()

# Given a list of sources, prepend the xrEngine source dir to each,
# and add it to the given target
function(target_engine_sources TARGET)
  foreach(src ${ARGN})
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/xrEngine/${src}")
  endforeach()
  target_sources(${TARGET} PRIVATE ${SOURCES})
endfunction()

# Given a list of precompiled headers, prepend the xrEngine source dir to each,
# and add it to the given target
function(target_engine_precompile_headers TARGET)
  foreach(src ${ARGN})
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/xrEngine/${src}")
  endforeach()
  target_precompile_headers(${TARGET} PRIVATE ${SOURCES})
endfunction()
