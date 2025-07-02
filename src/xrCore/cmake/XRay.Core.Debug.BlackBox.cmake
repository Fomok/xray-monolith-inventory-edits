add_core_static(XRay.Core.Debug.BlackBox
  PRIVATE
  blackbox/BlackBoxUI.cpp
  blackbox/BSUFunctions.cpp
  #blackbox/DiagAssert.cpp
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
)
target_folder(XRay.Core.Debug.BlackBox ${FOLDER_XRAY_CORE_DEBUG})

