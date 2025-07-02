add_library(XRay.Core.IntrusivePointer INTERFACE)

set_property(
  TARGET XRay.Core.IntrusivePointer
  PROPERTY FOLDER
  ${FOLDER_XRAY_CORE}
)

target_sources(XRay.Core.IntrusivePointer
  INTERFACE
  intrusive_ptr.h
  intrusive_ptr_inline.h
)
