# Add ./CMake to the module path and include NAME
macro(add_cmake_directory NAME)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
  include(${NAME})
endmacro()

# Add a library with the given NAME, and the following optional keyword parameters:
# SOURCES
# INCLUDES
# PRECOMPILES
# DEFINES
# LINKS
function(add_module NAME)
  # Parse keyword arguments
  cmake_parse_arguments(PARSE_ARGV 1 ARG
    "ROOT"
    "TYPE"
    "SOURCES;INCLUDES;PRECOMPILES;DEFINES;LINKS"
  )

  # Determine whether we have any C or CPP sources
  set(HAS_SOURCES false)
  if(ARG_SOURCES MATCHES "\\.cpp" OR ARG_SOURCES MATCHES "\\.c")
    set(HAS_SOURCES true)
  endif()

  # Store a flag if this module is explicitly set as an interface
  if(ARG_TYPE STREQUAL INTERFACE)
    set(EXPLICIT_INTERFACE true)
  endif()
  
  # If type has not been set explicitly, infer...
  if(NOT DEFINED ARG_TYPE)
    # If we have sources, this is a STATIC module, otherwise INTERFACE
    if(HAS_SOURCES)
      set(ARG_TYPE STATIC)
    else()
      set(ARG_TYPE INTERFACE)
    endif()
  endif()

  # If this is an interface module, override public and private semantics
  if(ARG_TYPE STREQUAL INTERFACE)
    set(TYPE_PUBLIC INTERFACE)
    set(TYPE_PRIVATE INTERFACE)
  else()
    set(TYPE_PUBLIC PUBLIC)
    set(TYPE_PRIVATE PRIVATE)
  endif()

  # Store semantics to parent scope for future reference
  set(${NAME}_TYPE_PUBLIC ${TYPE_PUBLIC} PARENT_SCOPE)
  set(${NAME}_TYPE_PRIVATE ${TYPE_PRIVATE} PARENT_SCOPE)

  # Add our module
  add_library(${NAME} ${ARG_TYPE})

  # Convert our module name into a folder path and apply it
  string(REPLACE "." ";" FOLDER ${NAME})
  if(NOT ARG_ROOT)
    list(POP_BACK FOLDER)
  endif()
  list(JOIN FOLDER "/" FOLDER)
  string(REPLACE "XRay" "X-Ray" FOLDER ${FOLDER})

  set_target_properties(${NAME}
    PROPERTIES
    FOLDER ${FOLDER}
  )

  # Convert our module name into a binary output path and apply it
  set(COMPILE_OUTPUT_DIR ${CMAKE_BINARY_DIR}/Binaries/$<CONFIG>)

  set_target_properties(${NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${COMPILE_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${COMPILE_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${COMPILE_OUTPUT_DIR}
    PDB_OUTPUT_DIRECTORY ${COMPILE_OUTPUT_DIR}
    COMPILE_PDB_OUTPUT_DIRECTORY ${COMPILE_OUTPUT_DIR}
  )

  # If we're not a root module, and aren't an explicit interface
  if(NOT ARG_ROOT AND NOT EXPLICIT_INTERFACE)
    # Decompose name into a path and fetch the first two segments
    string(REPLACE "." ";" PATH ${NAME})
    list(POP_FRONT PATH ROOT PARENT)
    # If we have a parent...
    if(PARENT)
      # Set ARG_CHILD_OF to our parent's path
      set(ARG_CHILD_OF ${ROOT}.${PARENT})
    endif()
  endif()

  # Compose sources
  target_sources(${NAME}
    PRIVATE
    ${ARG_SOURCES}
  )

  # Compose includes
  set(${NAME}_INCLUDES ${ARG_INCLUDES} PARENT_SCOPE)
  target_include_directories(${NAME}
    ${TYPE_PUBLIC}
    ${ARG_INCLUDES}
    ${${ARG_CHILD_OF}_INCLUDES}
  )

  # Compose precompiled headers
  set(${NAME}_PRECOMPILES ${ARG_PRECOMPILES} PARENT_SCOPE)
  target_precompile_headers(${NAME}
    ${TYPE_PRIVATE}
    ${ARG_PRECOMPILES}
    ${${ARG_CHILD_OF}_PRECOMPILES}
  )

  # Compose compile definitions
  set(${NAME}_DEFINES ${ARG_DEFINES} PARENT_SCOPE)
  target_compile_definitions(${NAME}
    ${TYPE_PUBLIC}
    ${ARG_DEFINES}
    ${${ARG_CHILD_OF}_DEFINES}
  )

  # Compose linked libraries
  set(${NAME}_LINKS ${ARG_LINKS} PARENT_SCOPE)
  target_link_libraries(${NAME}
    ${TYPE_PRIVATE}
    ${ARG_LINKS}
    ${${ARG_CHILD_OF}_LINKS}
  )

  # If we have a parent, link it to this module
  if(DEFINED ARG_CHILD_OF)
    target_link_libraries(${ARG_CHILD_OF} ${${ARG_CHILD_OF}_TYPE_PRIVATE} ${NAME})
  endif()
endfunction()