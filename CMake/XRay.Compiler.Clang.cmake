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
  
  -DWIN32
)

set(XRAY_COMPILER_FLAGS_DEBUG
  -fexceptions
)

set(XRAY_COMPILER_FLAGS_RELEASE
  -fno-exceptions
)
