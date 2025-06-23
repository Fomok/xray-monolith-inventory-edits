add_custom_target(serial)

set_property(
  TARGET serial
  PROPERTY FOLDER
  ${FOLDER_3RD_PARTY}
)

target_sources(serial
  PRIVATE
  tbb/parallel_for.h
  tbb/tbb_annotate.h
)
