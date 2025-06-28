include_guard()

## Manifest tool abstraction
macro(add_executable_manifest target manifest)
  if (WIN32)
    add_custom_command(
        TARGET ${target}
        POST_BUILD
      COMMAND mt -manifest ${manifest} -outputresource:$<TARGET_FILE:${target}>
        COMMENT "Adding manifest..." 
    )
  endif()
endmacro()
