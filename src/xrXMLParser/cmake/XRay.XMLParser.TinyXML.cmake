add_library(TinyXML STATIC)

set_property(
  TARGET TinyXML
  PROPERTY FOLDER
  ${FOLDER_EXTERNAL}
)

target_sources(TinyXML
  PRIVATE
  tinyxml.cpp
  tinyxmlerror.cpp
  tinyxmlparser.cpp
  
  tinyxml.h
)

target_precompile_headers(TinyXML
  PRIVATE
  stdafx.h
)

target_include_directories(TinyXML
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(TinyXML
  PRIVATE
  XRXMLPARSER_EXPORTS
)
