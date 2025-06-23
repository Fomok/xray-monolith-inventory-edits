add_custom_target(Include)

set_property(
  TARGET Include
  PROPERTY FOLDER
  ${FOLDER_XRAY}
)

target_sources(Include
  PRIVATE

  xrAPI/xrAPI.h

  xrRender/animation_blend.h
  xrRender/animation_motion.h
  xrRender/ApplicationRender.h
  xrRender/ConsoleRender.h
  xrRender/DebugRender.h
  xrRender/DebugShader.h
  xrRender/DrawUtils.h
  xrRender/EnvironmentRender.h
  xrRender/FactoryPtr.h
  xrRender/FontRender.h
  xrRender/ImGuiRender.h
  xrRender/Kinematics.h
  xrRender/KinematicsAnimated.h
  xrRender/LensFlareRender.h
  xrRender/ObjectSpaceRender.h
  xrRender/ParticleCustom.h
  xrRender/particles_systems_library_interface.hpp
  xrRender/RainRender.h
  xrRender/RenderDetailModel.h
  xrRender/RenderDeviceRender.h
  xrRender/RenderFactory.h
  xrRender/RenderVisual.h
  xrRender/StatGraphRender.h
  xrRender/StatsRender.h
  xrRender/ThunderboltDescRender.h
  xrRender/ThunderboltRender.h
  xrRender/UIRender.h
  xrRender/UISequenceVideoItem.h
  xrRender/UIShader.h
  xrRender/WallMarkArray.h

  R_light.h
)
