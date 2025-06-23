add_custom_target(loki)

set_property(
  TARGET loki
  PROPERTY FOLDER
  ${FOLDER_SDK}
)

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
