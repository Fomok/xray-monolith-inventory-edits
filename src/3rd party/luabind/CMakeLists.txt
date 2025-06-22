add_library(luabind STATIC)

set_property(
  TARGET luabind
  PROPERTY FOLDER
  ${FOLDER_LUA}
)

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

  luabind/adopt_policy.hpp
  luabind/back_reference.hpp
  luabind/back_reference_fwd.hpp
  luabind/class.hpp
  luabind/class_info.hpp
  luabind/com_policy.hpp
  luabind/config.hpp
  luabind/container_policy.hpp
  luabind/copy_policy.hpp
  luabind/dependency_policy.hpp
  luabind/detail/calc_arity.hpp
  luabind/detail/call.hpp
  luabind/detail/call_function.hpp
  luabind/detail/call_member.hpp
  luabind/detail/call_operator_iterate.hpp
  luabind/detail/class_cache.hpp
  luabind/detail/class_registry.hpp
  luabind/detail/class_rep.hpp
  luabind/detail/class_rep_scope.hpp
  luabind/detail/constructor.hpp
  luabind/detail/construct_rep.hpp
  luabind/detail/convert_to_lua.hpp
  luabind/detail/debug.hpp
  luabind/detail/decorate_type.hpp
  luabind/detail/enum_maker.hpp
  luabind/detail/find_best_match.hpp
  luabind/detail/garbage_collector.hpp
  luabind/detail/get_overload_signature.hpp
  luabind/detail/get_signature.hpp
  luabind/detail/implicit_cast.hpp
  luabind/detail/is_indirect_const.hpp
  luabind/detail/link_compatibility.hpp
  luabind/detail/method_rep.hpp
  luabind/detail/object_funs.hpp
  luabind/detail/object_rep.hpp
  luabind/detail/open.hpp
  luabind/detail/operators.hpp
  luabind/detail/operator_id.hpp
  luabind/detail/other.hpp
  luabind/detail/overload_rep.hpp
  luabind/detail/overload_rep_base.hpp
  luabind/detail/overload_rep_impl.hpp
  luabind/detail/pcall.hpp
  luabind/detail/pointee_sizeof.hpp
  luabind/detail/pointee_typeid.hpp
  luabind/detail/policy.hpp
  luabind/detail/primitives.hpp
  luabind/detail/property.hpp
  luabind/detail/ref.hpp
  luabind/detail/signature_match.hpp
  luabind/detail/stack_utils.hpp
  luabind/detail/typetraits.hpp
  luabind/detail/yes_no.hpp
  luabind/discard_result_policy.hpp
  luabind/error.hpp
  luabind/function.hpp
  luabind/functor.hpp
  luabind/iterator_policy.hpp
  luabind/library_linkage.h
  luabind/luabind.hpp
  luabind/luabind_auto_ptr.h
  luabind/luabind_delete.h
  luabind/luabind_memory.h
  luabind/luabind_memory_allocator.h
  luabind/luabind_memory_allocator_inline.h
  luabind/luabind_memory_manager_generator.h
  luabind/luabind_types.h
  luabind/lua_include.hpp
  luabind/object.hpp
  luabind/open.hpp
  luabind/operator.hpp
  luabind/out_value_policy.hpp
  luabind/prefix.hpp
  luabind/raw_policy.hpp
  luabind/return_reference_to_policy.hpp
  luabind/scope.hpp
  luabind/weak_ref.hpp
  luabind/wrapper_base.hpp
  luabind/yield_policy.hpp
  pch.h
)

target_include_directories(luabind
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/sdk/include"
  "${CMAKE_SOURCE_DIR}/src/3rd party/luajit-2/src"
)

target_precompile_headers(luabind
  PRIVATE
  pch.h
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
