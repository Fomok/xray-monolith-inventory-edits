add_library(XRay.XMLParser.TinyXML STATIC)

set_property(
  TARGET XRay.XMLParser.TinyXML
  PROPERTY FOLDER
  ${FOLDER_XRAY_XML_PARSER}
)

target_sources(XRay.XMLParser.TinyXML
  PRIVATE
  tinyxml.cpp
  tinyxmlerror.cpp
  tinyxmlparser.cpp
  
  tinyxml.h
)

target_precompile_headers(XRay.XMLParser.TinyXML
  PRIVATE
  stdafx.h
)

target_include_directories(XRay.XMLParser.TinyXML
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(XRay.XMLParser.TinyXML
  PRIVATE
  XRXMLPARSER_EXPORTS
)
