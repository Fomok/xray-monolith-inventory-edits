add_custom_target(stackwalker)

set_property(
  TARGET stackwalker
  PROPERTY FOLDER
  ${FOLDER_3RD_PARTY}
)

target_sources(stackwalker
  PRIVATE
  include/StackWalker.h
)
