include_guard()

# Configure compile definitions
add_compile_definitions(
  # Suppress deprecation errors
  _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
  # Activate verification for Verified builds
  $<$<CONFIG:Verified>:USE_VERIFY_IN_RELEASE>
)