set(XRPLATFORM_WINDOWS_H xrPlatform_Windows.h)

# Windows-specific definitions
add_module(XRay.Platform.Windows
  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}

  DEFINES
  VC_EXTRALEAN
  WIN32_LEAN_AND_MEAN
  STRICT
  IDIRECTPLAY2_OR_GREATER
  DIRECTINPUT_VERSION=0x0800
  _CRT_SECURE_NO_DEPRECATE

  # windows.h
  _WIN32_WINNT=0x0501

  NOGDICAPMASKS
  NOMENUS
  NOICONS
  NOKEYSTATES
  NODRAWTEXT
  NOMEMMGR
  NOMETAFILE
  NOSERVICE
  NOCOMM
  NOHELP
  NOPROFILER
  NOMCX
  NOMINMAX
  DOSWIN32
  _WIN32_DCOM
  _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

  SOURCES
  ${XRPLATFORM_WINDOWS_H}
)

# FIXME: Suppress warnings
target_compile_options(XRay.Platform.Windows
  INTERFACE
  ## Force include platform header
  $<$<CXX_COMPILER_ID:MSVC>:/FI${XRPLATFORM_WINDOWS_H}>
  $<$<CXX_COMPILER_ID:Clang>:-include${XRPLATFORM_WINDOWS_H}>
  $<$<CXX_COMPILER_ID:GNU>:-include${XRPLATFORM_WINDOWS_H}>

  ## Suppress MSVC warnings
  $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4251> # object needs DLL interface
  $<$<CXX_COMPILER_ID:MSVC>:/wd4201> # nonstandard extension used : nameless struct/union
  $<$<CXX_COMPILER_ID:MSVC>:/wd4100> # unreferenced formal parameter
  $<$<CXX_COMPILER_ID:MSVC>:/wd4127> # conditional expression is constant
  $<$<CXX_COMPILER_ID:MSVC>:/wd4530> # C++ exception handler used, but unwind semantics are not enabled
  $<$<CXX_COMPILER_ID:MSVC>:/wd4345>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4714> # __forceinline not inlined

  # local variable is initialized but not refenced
  # frequently in release code due to large amount of VERIFY
  $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:DEBUG>>:/wd4189>

  $<$<CXX_COMPILER_ID:MSVC>:/wd4512>
)

# On Windows, link the Windows platform module
target_link_libraries(XRay.Platform
  INTERFACE
  $<$<PLATFORM_ID:Windows>:XRay.Platform.Windows>
)