add_library(xrXMLParser STATIC)

target_sources(xrXMLParser PRIVATE
  StdAfx.cpp
  tinyxml.cpp
  tinyxmlerror.cpp
  tinyxmlparser.cpp
  xrXMLParser.cpp

  stdafx.h
  tinyxml.h
  xrXMLParser.h
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
