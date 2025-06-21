# Autoconfigured MSVC environment
# Allows MSVC tooling to be used during CMake configuration
# Based on https://izzys.casa/2023/09/finding-msvc-with-cmake/

include_guard(GLOBAL)

## Compiler definition abstractions

# Disable stdext::hash_map deprecation warnings
macro(silence_stdext_hash_deprecation_warnings TARGET)
  target_compile_definitions(${TARGET}
    PRIVATE
    _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
  )
endmacro()

## Find MSVC environment and expose via msvc.include / msvc.lib

# Ensure custom variables are copied to `try_compile` calls
set(CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
  XRAY_MSVS_VERSION
  XRAY_MSVS_EDITION
  XRAY_MSVS_TOOLSET
)

# If VS variables are not set, fall back to
# `CMAKE_HOST_SYSTEM_PROCESSOR` and `CMAKE_SYSTEM_PROCESSOR`
if (NOT CMAKE_GENERATOR MATCHES "^Visual Studio")
  if (NOT DEFINED CMAKE_SYSTEM_PROCESSOR)
    set(CMAKE_SYSTEM_PROCESSOR "${CMAKE_HOST_SYSTEM_PROCESSOR}")
  endif()
  if (NOT DEFINED CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE)
    set(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE "x86")
    if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64")
      set(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE "x64")
    elseif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "ARM64")
      set(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE "arm64")
    endif()
  endif()
  if (NOT DEFINED CMAKE_VS_PLATFORM_NAME)
    set(CMAKE_VS_PLATFORM_NAME "x86")
    if (CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
      set(CMAKE_VS_PLATFORM_NAME "x64")
    elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64")
      set(CMAKE_VS_PLATFORM_NAME "arm64")
    endif()
  endif()
endif()

# Find `vswhere`, allowing override via the `VSWHERE_EXECUTABLE` envvar
block(SCOPE_FOR VARIABLES)
  cmake_path(
    CONVERT "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer"
    TO_CMAKE_PATH_LIST vswhere.dir
    NORMALIZE)
  # This only temporarily affects the variable since we're inside a block.
  list(APPEND CMAKE_SYSTEM_PROGRAM_PATH "${vswhere.dir}")
  find_program(VSWHERE_EXECUTABLE NAMES vswhere DOC "Visual Studio Locator" REQUIRED)
endblock()

# Find Visual Studio installation
block(SCOPE_FOR VARIABLES)
  if (DEFINED XRAY_MSVS_EDITION)
    set(product "Microsoft.VisualStudio.Product.${XRAY_MSVS_EDITION}")
  else()
    set(product "*")
  endif()
  message(CHECK_START "Searching for Visual Studio ${XRAY_MSVS_EDITION}")
  execute_process(COMMAND "${VSWHERE_EXECUTABLE}" -nologo -nocolor
      -format json
      -products "${product}"
      -utf8
      -sort
    ENCODING UTF-8
    OUTPUT_VARIABLE candidates
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(JSON candidates.length LENGTH "${candidates}")
  string(JOIN " " error "Could not find Visual Studio"
  "${XRAY_MSVS_VERSION}"
  "${XRAY_MSVS_EDITION}")
  if (candidates.length EQUAL 0)
    message(CHECK_FAIL "no products")
    # You can choose to either hard fail here, or continue
    message(FATAL_ERROR "${error}")
  endif()

  if (NOT XRAY_MSVS_VERSION)
    string(JSON candidate.install.path GET "${candidates}" 0 "installationPath")
  else()
    # Unfortunately, range operations are inclusive in CMake for god knows why
    math(EXPR stop "${candidates.length} - 1")
    foreach (idx RANGE 0 ${stop})
      string(JSON version GET "${candidates}" ${idx} "catalog" "productLineVersion")
      if (version VERSION_EQUAL XRAY_MSVS_VERSION)
        string(JSON candidate.install.path 
          GET "${candidates}" ${idx} "installationPath")
        break()
      endif()
    endforeach()
  endif()
  if (NOT candidate.install.path)
    message(CHECK_FAIL "no install path found")
    message(FATAL_ERROR "${error}")
  endif()
  cmake_path(
    CONVERT "${candidate.install.path}"
    TO_CMAKE_PATH_LIST candidate.install.path
    NORMALIZE)
  message(CHECK_PASS "found : ${candidate.install.path}")
  set(XRAY_MSVS_INSTALL_PATH "${candidate.install.path}"
    CACHE PATH "Visual Studio Installation Path")
endblock()

# Find Windows SDK Root
message(CHECK_START "Searching for Windows SDK Root Directory")

cmake_host_system_information(RESULT XRAY_MSVS_WINDOWS_SDK_ROOT QUERY
  WINDOWS_REGISTRY "HKLM/SOFTWARE/Microsoft/Windows Kits/Installed Roots"
  VALUE "KitsRoot10"
  VIEW BOTH
  ERROR_VARIABLE error
)

if (error)
  message(CHECK_FAIL "not found : ${error}")
else()
  cmake_path(CONVERT "${XRAY_MSVS_WINDOWS_SDK_ROOT}"
    TO_CMAKE_PATH_LIST XRAY_MSVS_WINDOWS_SDK_ROOT
    NORMALIZE)
  set(XRAY_MSVS_WINDOWS_SDK_ROOT ${XRAY_MSVS_WINDOWS_SDK_ROOT}
    CACHE PATH "Windows SDK Installation Path")
  message(CHECK_PASS "found : ${XRAY_MSVS_WINDOWS_SDK_ROOT}")
endif()

# MSVC Tools / Windows SDK version directory lookup helper
function (msvs::directory out-var)
  if (${out-var})
    return()
  endif()

  cmake_parse_arguments(ARG "" "VARIABLE;PATH;DOC" "" ${ARGN})
  message(CHECK_START "Searching for ${ARG_DOC} in ${ARG_PATH}")
  # We want to get the list of options, but *not* the full path string, hence
  # the use of `RELATIVE`
  file(GLOB candidates
    LIST_DIRECTORIES YES
    RELATIVE "${ARG_PATH}"
    "${ARG_PATH}/*")
  
  list(SORT candidates COMPARE NATURAL ORDER DESCENDING)

  if (NOT DEFINED ${ARG_VARIABLE})
    list(GET candidates 0 ${out-var})
  else()
    foreach (candidate IN LISTS candidates)
      if ("${ARG_VARIABLE}" VERSION_EQUAL candidate)
        set(${out-var} "${candidate}")
        break()
      endif()
    endforeach()
  endif()
  if (NOT ${out-var})
    message(CHECK_FAIL "not found")
  else()
    message(CHECK_PASS "found : ${${out-var}}")
    set(${out-var} "${${out-var}}" CACHE INTERNAL "${out-var} value")
  endif()
endfunction()

cmake_language(CALL msvs::directory XRAY_MSVS_TOOLS_VERSION
  PATH "${XRAY_MSVS_INSTALL_PATH}/VC/Tools/MSVC"
  VARIABLE ${XRAY_MSVS_TOOLSET}
  DOC "MSVC Toolset")

# Your CMAKE_SYSTEM_VERSION should line up with the minimum SDK version you're
# targeting exactly.
cmake_language(CALL msvs::directory XRAY_MSVS_WINDOWS_SDK_VERSION
  PATH "${XRAY_MSVS_WINDOWS_SDK_ROOT}/Include"
  VARIABLE ${CMAKE_SYSTEM_VERSION}
  DOC "Windows SDK")

# Find the actual tools
set(windows.sdk.host "Host${CMAKE_GENERATOR_PLATFORM}")
set(windows.sdk.target "${CMAKE_GENERATOR_PLATFORM}")
set(msvc.tools.dir "${XRAY_MSVS_INSTALL_PATH}/VC/Tools/MSVC/${XRAY_MSVS_TOOLS_VERSION}")

block(SCOPE_FOR VARIABLES)
  list(PREPEND CMAKE_SYSTEM_PROGRAM_PATH
    "${msvc.tools.dir}/bin/${windows.sdk.host}/${windows.sdk.target}"
    "${XRAY_MSVS_WINDOWS_SDK_ROOT}/bin/${XRAY_MSVS_WINDOWS_SDK_VERSION}/${windows.sdk.target}"
    "${XRAY_MSVS_WINDOWS_SDK_ROOT}/bin")
  find_program(CMAKE_MASM_ASM_COMPILER NAMES ml64 ml DOC "MSVC ASM Compiler")
  find_program(CMAKE_CXX_COMPILER NAMES cl REQUIRED DOC "MSVC C++ Compiler")
  find_program(CMAKE_RC_COMPILER NAMES rc REQUIRED DOC "MSVC Resource Compiler")
  find_program(CMAKE_C_COMPILER NAMES cl REQUIRED DOC "MSVC C Compiler")
  find_program(CMAKE_LINKER NAMES link REQUIRED DOC "MSVC Linker")
  find_program(CMAKE_AR NAMES lib REQUIRED DOC "MSVC Archiver")
  find_program(CMAKE_MT NAMES mt REQUIRED DOC "MSVC Manifest Tool")
endblock()

set(includes ucrt shared um winrt cppwinrt)
set(libs ucrt um)

list(TRANSFORM includes PREPEND "${XRAY_MSVS_WINDOWS_SDK_ROOT}/Include/${XRAY_MSVS_WINDOWS_SDK_VERSION}/")

list(TRANSFORM lib PREPEND "${XRAY_MSVS_WINDOWS_SDK_ROOT}/Lib/${XRAY_MSVS_WINDOWS_SDK_VERSION}/")
list(TRANSFORM lib APPEND "/${windows.sdk.target}")

cmake_path(
    CONVERT "${msvc.tools.dir}/include"
    TO_NATIVE_PATH_LIST msvc.tools.include
    NORMALIZE)

cmake_path(
    CONVERT "${includes}"
    TO_NATIVE_PATH_LIST msvc.sdk.include
    NORMALIZE)

set(msvc.include ${msvc.tools.include} ${msvc.sdk.include})
set(msvc.lib ${lib})

