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

# Explicitly state when we're compiling for Win32
if(WIN32)
  list(APPEND XRAY_COMPILER_FLAGS -DWIN32)
endif()
