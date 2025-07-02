add_library(XRay.Core.IntrusivePointer INTERFACE)

target_folder(XRay.Core.IntrusivePointer ${FOLDER_XRAY_CORE})

target_sources(XRay.Core.IntrusivePointer
  INTERFACE
  intrusive_ptr.h
  intrusive_ptr_inline.h
)
