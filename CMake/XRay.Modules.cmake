# Add a library with the given NAME, and the following optional keyword parameters:
# SOURCES
# INCLUDES
# PRECOMPILES
# DEFINES
# LINKS
function(add_module NAME)
  # Parse keyword arguments
  cmake_parse_arguments(PARSE_ARGV 1 ARG
    ""
    "PARENT"
    "SOURCES;INCLUDES;PRECOMPILES;DEFINES;LINKS"
  )

  # Determine whether we have any C or CPP sources
  set(HAS_SOURCES false)
  if(ARG_SOURCES MATCHES "\\.cpp" OR ARG_SOURCES MATCHES "\\.c")
    set(HAS_SOURCES true)
  endif()
  
  # If we have sources, this is a STATIC module, otherwise INTERFACE
  set(TYPE INTERFACE)
  if(HAS_SOURCES)
    set(TYPE STATIC)
  endif()
  add_library(${NAME} ${TYPE})

  # Convert our module name into a folder path and apply it
  string(REPLACE "." ";" FOLDER ${NAME})
  list(POP_BACK FOLDER)
  list(JOIN FOLDER "/" FOLDER)
  string(REPLACE "XRay" "X-Ray" FOLDER ${FOLDER})

  set_target_properties(${NAME}
    PROPERTIES FOLDER
    ${FOLDER}
  )

  # Define a public-or-interface variable based on sources
  set(TYPE_PUBLIC INTERFACE)
  if(${HAS_SOURCES})
    set(TYPE_PUBLIC PUBLIC)
  endif()
  set(${NAME}_TYPE_PUBLIC ${TYPE_PUBLIC} PARENT_SCOPE)

  # Define a private-or-interface variable based on sources
  set(TYPE_PRIVATE INTERFACE)
  if(${HAS_SOURCES})
    set(TYPE_PRIVATE PRIVATE)
  endif()
  set(${NAME}_TYPE_PRIVATE ${TYPE_PRIVATE} PARENT_SCOPE)

  target_sources(${NAME}
    PRIVATE
    ${ARG_SOURCES}
  )

  # Compose includes
  set(${NAME}_INCLUDES ${ARG_INCLUDES} PARENT_SCOPE)
  target_include_directories(${NAME}
    ${TYPE_PUBLIC}
    ${ARG_INCLUDES}
    ${${ARG_PARENT}_INCLUDES}
  )

  # If we have sources...
  if(${HAS_SOURCES})
    # Compose precompiled headers
    set(${NAME}_PRECOMPILES ${ARG_PRECOMPILES} PARENT_SCOPE)
    target_precompile_headers(${NAME}
      PRIVATE
      ${ARG_PRECOMPILES}
      ${${ARG_PARENT}_PRECOMPILES}
    )

    # Compose compile definitions
    set(${NAME}_DEFINES ${ARG_DEFINES} PARENT_SCOPE)
    target_compile_definitions(${NAME}
      PRIVATE
      ${ARG_DEFINES}
      ${${ARG_PARENT}_DEFINES}
    )
  endif()

  # Compose linked libraries
  set(${NAME}_LINKS ${ARG_LINKS} PARENT_SCOPE)
  target_link_libraries(${NAME}
    ${TYPE_PRIVATE}
    ${ARG_LINKS}
    ${${ARG_PARENT}_LINKS}
  )

  # If we have a parent, link it to this module
  if(DEFINED ARG_PARENT)
    target_link_libraries(${ARG_PARENT} ${${ARG_PARENT}_TYPE_PRIVATE} ${NAME})
  endif()
endfunction()