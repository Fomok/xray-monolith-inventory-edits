add_module(XRay.XMLParser
  TYPE STATIC
  
  PRECOMPILES stdafx.h
  
  INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}

  DEFINES
  XRXMLPARSER_EXPORTS

  LINKS
  TinyXML
  
  SOURCES
  xrXMLParser.cpp
  xrXMLParser.h
)

include(TinyXML)