add_library(xrXMLParser STATIC)

target_sources_grouped(
  TARGET xrXMLParser
  NAME "Kernel/Source Files"
  FILES
  StdAfx.cpp
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
  PRIVATE stdafx.h
)

target_include_directories(xrXMLParser
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(xrXMLParser
  PUBLIC
  NDEBUG
  XRXMLPARSER_EXPORTS

  PRIVATE
  WIN32
  _WINDOWS
  _SECURE_SCL=0
)

enable_parallel_build(xrXMLParser)
disable_exceptions()
silence_stdext_hash_deprecation_warnings(xrXMLParser)
