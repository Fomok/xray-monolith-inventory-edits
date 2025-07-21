add_module(XRay.XMLParser
  TYPE STATIC
  
  PRECOMPILES
  [["xrCore.h"]]
  xrXMLParser.h
  
  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}

  DEFINES
  XRCORE_EXPORTS
  XRXMLPARSER_EXPORTS

  DEPENDS
  XRay.Core
  XRay.Render.Common

  LINKS
  TinyXML
  
  SOURCES
  xrXMLParser.cpp
  xrXMLParser.h
)
