include_guard()

set(XRAY_COMPILER_FLAGS
  # Compatibility flags
  -Wno-implicit-function-declaration
  -Wno-c++11-narrowing
  -Wno-invalid-token-paste
  -Wno-nonportable-include-path
  -Wno-shift-negative-value
  -Wno-address-of-temporary
  -Wno-register

  # Initialize to zero for MSVC equivalence
  -ftrivial-auto-var-init=zero
  # ...but warn about it
  -Wuninitialized
)

if(WIN32)
  # Explicitly state that we're compiling for Win32
  set(XRAY_COMPILER_FLAGS "${XRAY_COMPILER_FLAGS};-DWIN32")
endif()

set(XRAY_COMPILER_FLAGS_DEBUG
  -fexceptions
)

set(XRAY_COMPILER_FLAGS_RELEASE
  -fno-exceptions
)
