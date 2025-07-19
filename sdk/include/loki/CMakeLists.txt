add_library(loki INTERFACE)

target_folder(loki ${FOLDER_SDK})

target_sources(loki
  PRIVATE
  AbstractFactory.h
  AssocVector.h
  CMakeLists.txt
  DataGenerators.h
  EmptyType.h
  Factory.h
  Functor.h
  HierarchyGenerators.h
  LokiTypeInfo.h
  MinMax.h
  MultiMethods.h
  NullType.h
  Singleton.cpp
  Singleton.h
  SmallObj.cpp
  SmallObj.h
  SmartPtr.h
  static_check.h
  Threads.h
  Typelist.h
  TypeManip.h
  TypeTraits.h
  Variant.h
  VC_Alignment.h
  Visitor.h
)

target_compile_options(loki
  INTERFACE
  $<$<CXX_COMPILER_ID:Clang>:-Wno-microsoft-template-shadow>
)

target_include_directories(loki
  INTERFACE
  ${XRAY_SDK_INCLUDE}
)
