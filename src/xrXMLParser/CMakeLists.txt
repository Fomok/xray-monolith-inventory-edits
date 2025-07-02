include(cmake/XRay.XMLParser.TinyXML.cmake)

add_library(XRay.XMLParser STATIC)

set_property(
  TARGET XRay.XMLParser
  PROPERTY FOLDER
  ${FOLDER_XRAY_XML_PARSER}
)

target_sources(XRay.XMLParser
  PRIVATE
  xrXMLParser.cpp
  xrXMLParser.h
)

target_precompile_headers(XRay.XMLParser
  PRIVATE
  ../xrCore/xrCore.h
)

target_include_directories(XRay.XMLParser
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(XRay.XMLParser
  PRIVATE
  XRXMLPARSER_EXPORTS
)

target_link_libraries(XRay.XMLParser
  PRIVATE
  TinyXML
)