add_library(xrXMLParser.TinyXML STATIC)

set_property(
  TARGET xrXMLParser.TinyXML
  PROPERTY FOLDER
  ${FOLDER_XRAY_XML_PARSER}
)

target_sources(xrXMLParser.TinyXML
  PRIVATE
  tinyxml.cpp
  tinyxmlerror.cpp
  tinyxmlparser.cpp
  
  tinyxml.h
)

target_precompile_headers(xrXMLParser.TinyXML
  PRIVATE
  stdafx.h
)

target_include_directories(xrXMLParser.TinyXML
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(xrXMLParser.TinyXML
  PRIVATE
  XRXMLPARSER_EXPORTS
)
