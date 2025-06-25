add_library(xrXMLParser STATIC)

set_property(
  TARGET xrXMLParser
  PROPERTY FOLDER
  ${FOLDER_XRAY}
)

target_sources_grouped(
  TARGET xrXMLParser
  NAME "Kernel/Source Files"
  FILES
  xrXMLParser.cpp
)

target_sources_grouped(
  TARGET xrXMLParser
  NAME "Kernel/Header Files"
  FILES
  stdafx.h
  xrXMLParser.h
)

target_sources_grouped(
  TARGET xrXMLParser
  NAME "TinyXML/Source Files"
  FILES
  tinyxml.cpp
  tinyxmlerror.cpp
  tinyxmlparser.cpp
)

target_sources_grouped(
  TARGET xrXMLParser
  NAME "TinyXML/Header Files"
  FILES
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
