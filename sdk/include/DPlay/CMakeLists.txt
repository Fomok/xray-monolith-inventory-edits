add_custom_target(DPlay)

set_property(
  TARGET DPlay
  PROPERTY FOLDER
  ${FOLDER_SDK}
)

target_sources(DPlay
  PRIVATE
  dpaddr.h
  dplay8.h
)
