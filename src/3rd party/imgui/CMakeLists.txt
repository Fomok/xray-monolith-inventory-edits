add_library(imgui STATIC)

set_property(
  TARGET imgui
  PROPERTY FOLDER
  ${FOLDER_3RD_PARTY}
)

target_sources(imgui PRIVATE
  IconsFontAwesome6.h
  imconfig.h
  imgui.cpp
  imgui.h
  imgui.natvis
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_internal.h
  imgui_tables.cpp
  imgui_widgets.cpp
  imstb_rectpack.h
  imstb_textedit.h
  imstb_truetype.h
)

target_sources(imgui
  PRIVATE
  imgui_custom.cpp
  backends/imgui_impl_dx10.cpp
  backends/imgui_impl_dx11.cpp
  backends/imgui_impl_dx9.cpp

  imgui_custom.h
  backends/imgui_impl_dx10.h
  backends/imgui_impl_dx11.h
  backends/imgui_impl_dx9.h
)

target_include_directories(imgui
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  "${CMAKE_SOURCE_DIR}/src/3rd party"
)
