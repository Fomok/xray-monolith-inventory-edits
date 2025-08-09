add_module(XRay.Render.DX10
  TYPE INTERFACE

  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}

  SOURCES
  3DFluid/dx103DFluidBlenders.cpp
  3DFluid/dx103DFluidBlenders.h
  3DFluid/dx103DFluidData.cpp
  3DFluid/dx103DFluidData.h
  3DFluid/dx103DFluidEmitters.cpp
  3DFluid/dx103DFluidEmitters.h
  3DFluid/dx103DFluidGrid.cpp
  3DFluid/dx103DFluidGrid.h
  3DFluid/dx103DFluidManager.cpp
  3DFluid/dx103DFluidManager.h
  3DFluid/dx103DFluidObstacles.cpp
  3DFluid/dx103DFluidObstacles.h
  3DFluid/dx103DFluidRenderer.cpp
  3DFluid/dx103DFluidRenderer.h
  3DFluid/dx103DFluidVolume.cpp
  3DFluid/dx103DFluidVolume.h

  "DX10 Rain/dx10RainBlender.cpp"
  "DX10 Rain/dx10RainBlender.h"

  MSAA/dx10MSAABlender.cpp
  MSAA/dx10MSAABlender.h

  StateManager/dx10SamplerStateCache.cpp
  StateManager/dx10SamplerStateCache.h
  StateManager/dx10ShaderResourceStateCache.cpp
  StateManager/dx10ShaderResourceStateCache.h
  StateManager/dx10State.cpp
  StateManager/dx10State.h
  StateManager/dx10StateCache.cpp
  StateManager/dx10StateCache.h
  StateManager/dx10StateCacheImpl.h
  StateManager/dx10StateManager.cpp
  StateManager/dx10StateManager.h

  Blender_Recorder_R3.cpp
  dx10BufferUtils.cpp
  dx10BufferUtils.h
  dx10ConstantBuffer.cpp
  dx10ConstantBuffer.h
  dx10ConstantBuffer_impl.h
  dx10DetailManager_VS.cpp
  dx10HW.cpp
  dx10R_Backend_Runtime.h
  dx10r_constants.cpp
  dx10r_constants_cache.cpp
  dx10r_constants_cache.h
  dx10ResourceManager_Resources.cpp
  dx10ResourceManager_Scripting.cpp
  dx10SH_RT.cpp
  dx10SH_Texture.cpp
  dx10StateUtils.cpp
  dx10StateUtils.h
  dx10Texture.cpp
  dx10TextureUtils.cpp
  dx10TextureUtils.h
  DXCommonTypes.h
)
