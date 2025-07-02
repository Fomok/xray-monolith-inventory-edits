function(add_module NAME)
  string(REPLACE "." ";" FOLDER ${NAME})
  list(POP_BACK FOLDER)
  list(JOIN FOLDER "/" FOLDER)
  string(REPLACE "XRay" "X-Ray" FOLDER ${FOLDER})
  
  cmake_parse_arguments(PARSE_ARGV 1 ARG
    ""
    "PARENT"
    "SOURCES;INCLUDES;PCH;DEFS;LIBS"
  )

  set(IS_INTERFACE true)
  if(ARG_SOURCES MATCHES ".cpp" OR ARG_SOURCES MATCHES ".c")
    set(IS_INTERFACE false)
  endif()
  
  set(TYPE STATIC)
  if(IS_INTERFACE)
    set(TYPE INTERFACE)
  endif()
  add_library(${NAME} ${TYPE})

  set_target_properties(${NAME}
    PROPERTIES FOLDER
    ${FOLDER}
  )

  set(TYPE_PUBLIC PUBLIC)
  if(${IS_INTERFACE})
    set(TYPE_PUBLIC INTERFACE)
  endif()

  set(TYPE_PRIVATE PRIVATE)
  if(${IS_INTERFACE})
    set(TYPE_PRIVATE INTERFACE)
  endif()

  target_sources(${NAME}
    PRIVATE
    ${ARG_SOURCES}
  )

  set(${NAME}_INCLUDES ${ARG_INCLUDES} PARENT_SCOPE)
  target_include_directories(${NAME}
    ${TYPE_PUBLIC}
    ${ARG_INCLUDES}
    ${${ARG_PARENT}_INCLUDES}
  )

  if(NOT ${IS_INTERFACE})
    set(${NAME}_PCH ${ARG_PCH} PARENT_SCOPE)
    target_precompile_headers(${NAME}
      PRIVATE
      ${ARG_PCH}
      ${${ARG_PARENT}_PCH}
    )

    set(${NAME}_DEFS ${ARG_DEFS} PARENT_SCOPE)
    target_compile_definitions(${NAME}
      PRIVATE
      ${ARG_DEFS}
      ${${ARG_PARENT}_DEFS}
    )
  endif()

  set(${NAME}_LIBS ${ARG_LIBS} PARENT_SCOPE)
  target_link_libraries(${NAME}
    ${TYPE_PRIVATE}
    ${ARG_LIBS}
    ${${ARG_PARENT}_LIBS}
  )

  if(DEFINED ARG_PARENT)
    target_link_libraries(${ARG_PARENT} PRIVATE ${NAME})
  endif()
endfunction()