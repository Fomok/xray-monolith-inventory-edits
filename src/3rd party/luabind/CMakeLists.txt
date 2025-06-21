add_library(luabind STATIC)

set_target_properties(luabind PROPERTIES
  OUTPUT_NAME "luabind.beta7-devel.rc4"
)

target_sources(luabind PRIVATE
    library_linkage.cpp
    luabind_memory.cpp
    pch.cpp
    src/class.cpp
    src/class_info.cpp
    src/class_registry.cpp
    src/class_rep.cpp
    src/create_class.cpp
    src/error.cpp
    src/find_best_match.cpp
    src/function.cpp
    src/implicit_cast.cpp
    src/link_compatibility.cpp
    src/object.cpp
    src/object_rep.cpp
    src/open.cpp
    src/pcall.cpp
    src/ref.cpp
    src/scope.cpp
    src/stack_content_by_name.cpp
    src/weak_ref.cpp
    src/wrapper_base.cpp
)

target_include_directories(luabind
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/sdk/include"
  "${CMAKE_SOURCE_DIR}/src/3rd party/luajit-2/src"
)

target_compile_definitions(luabind
  PUBLIC
  NDEBUG
  LUABIND_BUILDING

  PRIVATE
  WIN32
  _SECURE_SCL=0
  _HAS_ITERATOR_DEBUGGING=0
)

enable_parallel_build(luabind)
