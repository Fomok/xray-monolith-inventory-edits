add_library(ReShadeCompat STATIC)

target_folder(ReShadeCompat ${FOLDER_EXTERNAL})

target_sources(ReShadeCompat
  PRIVATE
  addon.cpp
)

target_precompile_headers(ReShadeCompat
  PRIVATE
  <imgui/imgui.h>
  <reshade/reshade.hpp>
)

target_compile_options(ReShadeCompat
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

target_compile_definitions(ReShadeCompat
  PRIVATE
  PURE_ALLOC
  IMGUI_DISABLE_INCLUDE_IMCONFIG_H
  ImTextureID=ImU64
)

target_link_libraries(ReShadeCompat
  PRIVATE
  imgui
  reshade
  XRay.Core.Defines
)