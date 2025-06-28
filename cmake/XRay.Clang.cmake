include_guard()

add_compile_options(
  # Compatibility flags
  -Wno-implicit-function-declaration
  -Wno-c++11-narrowing
  -Wno-invalid-token-paste
  -Wno-nonportable-include-path
  -Wno-shift-negative_value

  # Configure exceptions
  $<$<NOT:$<CONFIG:Debug>>:-fno-exceptions>
)
