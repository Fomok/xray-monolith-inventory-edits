add_module(XRay.XMLParser
  TYPE STATIC
  
  PRECOMPILES
  <xrCore.h>
  xrXMLParser.h
  
  INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}

  DEFINES
  XRXMLPARSER_EXPORTS

  LINKS
  XRay.Core
  TinyXML
  
  SOURCES
  xrXMLParser.cpp
  xrXMLParser.h
)
