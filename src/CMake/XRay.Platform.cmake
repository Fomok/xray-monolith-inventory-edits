set(XRPLATFORM_H xrPlatform.h)

# Single source of truth for global definitions
add_module(XRay.Platform
  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}

  LINKS
  XRay.Includes

  SOURCES
  ${XRPLATFORM_H}
)

# Force include platform header
target_compile_options(XRay.Platform
  INTERFACE
  $<$<CXX_COMPILER_ID:MSVC>:/FI${XRPLATFORM_H}>
  $<$<CXX_COMPILER_ID:Clang>:-include${XRPLATFORM_H}>
  $<$<CXX_COMPILER_ID:GNU>:-include${XRPLATFORM_H}>
)

# Platform-specific submodules
include(XRay.Platform.Windows)

# Link to all subsequent libraries, i.e. XRay.*
# Externals will need to link XRay.Platform manually
link_libraries(XRay.Platform)