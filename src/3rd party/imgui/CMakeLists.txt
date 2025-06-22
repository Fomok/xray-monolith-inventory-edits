add_library(imgui STATIC)

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

target_sources_grouped(
  TARGET imgui
  NAME "Backends/Source Files"
  FILES
  imgui_custom.cpp
  backends/imgui_impl_dx10.cpp
  backends/imgui_impl_dx11.cpp
  backends/imgui_impl_dx9.cpp
)

target_sources_grouped(
  TARGET imgui
  NAME "Backends/Header Files"
  FILES
  imgui_custom.h
  backends/imgui_impl_dx10.h
  backends/imgui_impl_dx11.h
  backends/imgui_impl_dx9.h
)

target_include_directories(imgui
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(imgui
  PUBLIC
  NDEBUG

  PRIVATE
  _CONSOLE
)

enable_parallel_build(imgui)
