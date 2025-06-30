add_library(xrXMLParser STATIC)

set_property(
  TARGET xrXMLParser
  PROPERTY FOLDER
  ${FOLDER_XRAY}
)

target_sources(xrXMLParser
  PRIVATE
  xrXMLParser.cpp
  
  stdafx.h
  xrXMLParser.h
  
  tinyxml.cpp
  tinyxmlerror.cpp
  tinyxmlparser.cpp
  
  tinyxml.h
)

target_precompile_headers(xrXMLParser
  PRIVATE
  stdafx.h
)

target_include_directories(xrXMLParser
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(xrXMLParser
  PRIVATE
  XRXMLPARSER_EXPORTS
)
