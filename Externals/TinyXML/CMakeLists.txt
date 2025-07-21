add_library(TinyXML STATIC)

target_folder(TinyXML ${FOLDER_EXTERNAL})

target_include_directories(TinyXML
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${DEPENDS_XRay.Core}
  ${DEPENDS_XRay.XMLParser}
)

target_compile_definitions(TinyXML
  PRIVATE
  XRCORE_EXPORTS
  XRXMLPARSER_EXPORTS
)

target_precompile_headers(TinyXML
  PRIVATE
  <xrCore.h>
  <xrXMLParser.h>
)

target_sources(TinyXML
  PRIVATE
  tinyxml.cpp
  tinyxmlerror.cpp
  tinyxmlparser.cpp
  
  tinyxml.h
)