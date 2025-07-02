add_library(XRay.Core STATIC)

set_property(
  TARGET XRay.Core
  PROPERTY FOLDER
  ${FOLDER_XRAY_CORE}
)

target_sources(XRay.Core
  PRIVATE
  LzHuf.cpp
  lzhuf.h
  
  lzo_compressor.cpp
  rt_lzo1x_1.cpp
  rt_lzo1x_9x.cpp
  rt_lzo1x_d1.cpp
  rt_lzo1x_d2.cpp
  rt_lzo1x_d3.cpp
  rt_lzo_init.cpp
  
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
  
  PPMd.h
  PPMdType.h
  
  Model.cpp
  Coder.hpp
  
  SubAlloc.hpp
  
  compression_ppmd_stream.h
  compression_ppmd_stream_inline.h
  
  ppmd_compressor.cpp
  
  ppmd_compressor.h
  
  rt_compressor.cpp
  rt_compressor9.cpp
  
  rt_compressor.h
  
  blackbox/BlackBoxUI.cpp
  blackbox/BSUFunctions.cpp
  blackbox/CrashHandler.cpp
  blackbox/GetLoadedModules.cpp
  blackbox/IsNT.cpp
  blackbox/NT4ProcessInfo.cpp
  blackbox/TLHELPProcessInfo.cpp
  
  blackbox/BugslayerUtil.h
  blackbox/CrashHandler.h
  blackbox/DiagAssert.h
  blackbox/Internal.h
  blackbox/stdafx_.h
  blackbox/SymbolEngine.h
  blackbox/WarningsOff.h
  blackbox/WarningsOn.h
  
  #xrDebug.cpp
  xrDebugNew.cpp
  
  file_stream_reader.cpp
  
  file_stream_reader.h
  
  stream_reader.cpp
  
  stream_reader.h
  stream_reader_inline.h
  
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
  
  intrusive_ptr.h
  intrusive_ptr_inline.h
  
  FTimer.cpp
  xrCore.cpp
  
  FTimer.h
  stdafx.h
  xrCore.h
  xrCore_platform.h
  
  cpuid.cpp
  
  cpuid.h
  
  buffer_vector.h
  buffer_vector_inline.h
  
  _std_extensions.cpp
  clsid.cpp
  string_concatenations.cpp
  xr_trims.cpp
  
  _std_extensions.h
  _stl_extensions.h
  _type_traits.h
  clsid.h
  fastdelegate.h
  string_concatenations.h
  string_concatenations_inline.h
  xr_trims.h
  
  FixedMap.h
  FixedSet.h
  FixedVector.h
  xrPool.h
  dump_string.h
  
  _compressed_normal.cpp
  _math.cpp
  _sphere.cpp
  dump_string.cpp
  
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
  
  xrMemory.h
  xrMemory_align.h
  xrMEMORY_POOL.h
  xrMemory_pso.h
  xrMemory_pure.h
  xrMemory_subst_borland.h
  xrMemory_subst_msvc.h
  
  memory_monitor.cpp
  
  memory_monitor.h
  
  os_clipboard.cpp
  
  os_clipboard.h
  
  profiler.h
  
  crc32.cpp
  mezz_stringbuffer.cpp
  xr_shared.cpp
  xrsharedmem.cpp
  xrstring.cpp
  
  mezz_stringbuffer.h
  xr_resource.h
  xr_shared.h
  xrsharedmem.h
  xrstring.h
  
  Lock.cpp
  ScopeLock.cpp
  xrSyncronize.cpp
  
  Lock.hpp
  ScopeLock.hpp
  xrSyncronize.h
  
  ../build_config_defines.h
  ChooseTypes.H
  client_id.h
  robin_hood.h

  resource.h
  xrCore.rc
)

target_include_directories(XRay.Core
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(XRay.Core
  PRIVATE
  stdafx.h
)

target_compile_definitions(XRay.Core
  PRIVATE
  PURE_ALLOC
  XRCORE_EXPORTS
  PORTABLE_BUGSLAYERUTIL
)

target_link_libraries(XRay.Core
  PRIVATE
  DXERR
  optick
  StackWalker
  winmm
  XRay.Render.API
  XRay.CDB
)
