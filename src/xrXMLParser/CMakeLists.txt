include(cmake/XRay.XMLParser.TinyXML.cmake)

add_library(xrXMLParser STATIC)

set_property(
  TARGET xrXMLParser
  PROPERTY FOLDER
  ${FOLDER_XRAY_XML_PARSER}
)

target_sources(xrXMLParser
  PRIVATE
  xrXMLParser.cpp
  
  stdafx.h
  xrXMLParser.h
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

target_link_libraries(xrXMLParser
  PRIVATE
  xrXMLParser.TinyXML
)