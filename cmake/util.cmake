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

# Disable CXX exceptions for the current scope
macro(disable_exceptions)
  string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endmacro()
