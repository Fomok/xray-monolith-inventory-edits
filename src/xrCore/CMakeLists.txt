add_library(xrCore STATIC)

set_property(
  TARGET xrCore
  PROPERTY FOLDER
  ${FOLDER_XRAY}
)

target_sources_grouped(
  TARGET xrCore
  NAME "Compression/LZ/Source Files"
  FILES
  LzHuf.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Compression/LZ/Header Files"
  FILES
  lzhuf.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Compression/LZO/Source Files"
  FILES
  lzo_compressor.cpp
  rt_lzo1x_1.cpp
  rt_lzo1x_9x.cpp
  rt_lzo1x_d1.cpp
  rt_lzo1x_d2.cpp
  rt_lzo1x_d3.cpp
  rt_lzo_init.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Compression/LZO/Header Files"
  FILES
  lzo_compressor.h
  rt_config1x.h
  rt_lzo1x.h
  rt_lzoconf.h
  rt_lzodefs.h
  rt_lzo_conf.h
  rt_lzo_config.h
  rt_lzo_dict.h
  rt_lzo_ptr.h
  rt_miniacc.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Compression/PPMd/Common/Header Files"
  FILES
  PPMd.h
  PPMdType.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Compression/PPMd/Core/Source Files"
  FILES
  Model.cpp
  Coder.hpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Compression/PPMd/Core/Header Files"
  FILES
)

target_sources_grouped(
  TARGET xrCore
  NAME "Compression/PPMd/Core/Allocator"
  FILES
  SubAlloc.hpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Compression/PPMd/Stream"
  FILES
  compression_ppmd_stream.h
  compression_ppmd_stream_inline.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Compression/PPMd/Source Files"
  FILES
  ppmd_compressor.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Compression/PPMd/Header Files"
  FILES
  ppmd_compressor.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Compression/RT/Source Files"
  FILES
  rt_compressor.cpp
  rt_compressor9.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Compression/RT/Header Files"
  FILES
  rt_compressor.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Debug Core/BlackBox/Source Files"
  FILES
  blackbox/BlackBoxUI.cpp
  blackbox/BSUFunctions.cpp
  blackbox/CrashHandler.cpp
  blackbox/GetLoadedModules.cpp
  blackbox/IsNT.cpp
  blackbox/NT4ProcessInfo.cpp
  blackbox/TLHELPProcessInfo.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Debug Core/BlackBox/Header Files"
  FILES
  blackbox/BugslayerUtil.h
  blackbox/CrashHandler.h
  blackbox/DiagAssert.h
  blackbox/Internal.h
  blackbox/stdafx_.h
  blackbox/SymbolEngine.h
  blackbox/WarningsOff.h
  blackbox/WarningsOn.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Debug Core/Source Files"
  FILES
  #xrDebug.cpp
  xrDebugNew.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "FS/File Stream Reader/Source Files"
  FILES
  file_stream_reader.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "FS/File Stream Reader/Header Files"
  FILES
  file_stream_reader.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "FS/File Stream/Source Files"
  FILES
  stream_reader.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "FS/File Stream/Header Files"
  FILES
  stream_reader.h
  stream_reader_inline.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "FS/Source Files"
  FILES
  FileSystem.cpp
  FileSystem_borland.cpp
  FS.cpp
  LocatorAPI.cpp
  LocatorAPI_auth.cpp
  LocatorAPI_defs.cpp
  #LocatorAPI_Notifications.cpp
  log.cpp
  NET_utils.cpp
  Xr_ini.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "FS/Header Files"
  FILES
  FileSystem.h
  FS.h
  FS_impl.h
  FS_internal.h
  LocatorAPI.h
  LocatorAPI_defs.h
  #LocatorAPI_Notifications.h
  log.h
  net_utils.h
  xr_ini.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Intrusive Pointer"
  FILES
  intrusive_ptr.h
  intrusive_ptr_inline.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Kernel/Source Files"
  FILES
  FTimer.cpp
  xrCore.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Kernel/Header Files"
  FILES
  FTimer.h
  stdafx.h
  xrCore.h
  xrCore_platform.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Math/CPU/Source Files"
  FILES
  cpuid.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Math/CPU/Header Files"
  FILES
  cpuid.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Math/Extensions/Buffer Vector"
  FILES
  buffer_vector.h
  buffer_vector_inline.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Math/Extensions/Source Files"
  FILES
  _std_extensions.cpp
  clsid.cpp
  string_concatenations.cpp
  xr_trims.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Math/Extensions/Header Files"
  FILES
  _std_extensions.h
  _stl_extensions.h
  _type_traits.h
  clsid.h
  fastdelegate.h
  string_concatenations.h
  string_concatenations_inline.h
  xr_trims.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Math/STLext"
  FILES
  FixedMap.h
  FixedSet.h
  FixedVector.h
  xrPool.h
  dump_string.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Math/Source Files"
  FILES
  _compressed_normal.cpp
  _math.cpp
  _sphere.cpp
  dump_string.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Math/Header Files"
  FILES
  _bitwise.h
  _color.h
  _compressed_normal.h
  _cylinder.h
  _fbox.h
  _fbox2.h
  _flags.h
  _math.h
  _matrix.h
  _matrix33.h
  _obb.h
  _plane.h
  _plane2.h
  _quaternion.h
  _random.h
  _rect.h
  _sphere.h
  _types.h
  _vector2.h
  _vector3d.h
  _vector3d_ext.h
  _vector4.h
  vector.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Memory Manager/Source Files"
  FILES
  memory_allocation_stats.cpp
  memory_usage.cpp
  xrMemory.cpp
  xrMemory_align.cpp
  xrMemory_debug.cpp
  xrMemory_POOL.cpp
  xrMemory_pso_Copy.cpp
  xrMemory_pso_Fill.cpp
  xrMemory_pso_Fill32.cpp
  xrMemory_subst_borland.cpp
  xrMemory_subst_msvc.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Memory Manager/Header Files"
  FILES
  xrMemory.h
  xrMemory_align.h
  xrMEMORY_POOL.h
  xrMemory_pso.h
  xrMemory_pure.h
  xrMemory_subst_borland.h
  xrMemory_subst_msvc.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Memory Monitor/Source Files"
  FILES
  memory_monitor.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Memory Monitor/Header Files"
  FILES
  memory_monitor.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "OS/Source Files"
  FILES
  os_clipboard.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "OS/Header Files"
  FILES
  os_clipboard.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Profling"
  FILES
  profiler.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Shared Memory & String Library/Source Files"
  FILES
  crc32.cpp
  mezz_stringbuffer.cpp
  xr_shared.cpp
  xrsharedmem.cpp
  xrstring.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Shared Memory & String Library/Header Files"
  FILES
  mezz_stringbuffer.h
  xr_resource.h
  xr_shared.h
  xrsharedmem.h
  xrstring.h
)

target_sources_grouped(
  TARGET xrCore
  NAME "Threading/Source Files"
  FILES
  Lock.cpp
  ScopeLock.cpp
  xrSyncronize.cpp
)

target_sources_grouped(
  TARGET xrCore
  NAME "Threading/Header Files"
  FILES
  Lock.hpp
  ScopeLock.hpp
  xrSyncronize.h
)

target_sources(xrCore
  PRIVATE
  ../build_config_defines.h
  ChooseTypes.H
  client_id.h
  robin_hood.h

  #resource.h
  xrCore.rc
)

target_include_directories(xrCore
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(xrCore
  PRIVATE
  stdafx.h
  blackbox/stdafx_.h
)

target_compile_definitions(xrCore
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(xrCore
  PUBLIC
  DXERR
  StackWalker
  winmm
  xrAPI
  xrCDB
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Profiled")
  target_link_libraries(xrCore PUBLIC optick)
endif()
