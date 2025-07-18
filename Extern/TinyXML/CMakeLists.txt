add_library(TinyXML STATIC)

target_folder(TinyXML ${FOLDER_EXTERNAL})

target_sources(TinyXML
  PRIVATE
  tinyxml.cpp
  tinyxmlerror.cpp
  tinyxmlparser.cpp
  
  tinyxml.h
)

target_precompile_headers(TinyXML
  PRIVATE
  <xrCore.h>
  <xrXMLParser.h>
)

target_include_directories(TinyXML
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(TinyXML
  PRIVATE
  XRXMLPARSER_EXPORTS
)

target_link_libraries(TinyXML
  PRIVATE
  XRay.Core
  XRay.XMLParser
)