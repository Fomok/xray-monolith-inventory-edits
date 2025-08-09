add_module(XRay.XMLParser
  TYPE STATIC
  
  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}

  DEFINES
  XRXMLPARSER_EXPORTS

  LINKS
  FastDynamicCast
  TinyXML

  XRay.Core.Defines
  
  XRay.Core.Includes
  
  SOURCES
  xrXMLParser.cpp
  xrXMLParser.h
)
