add_library(imgui STATIC)

target_folder(imgui ${FOLDER_EXTERNAL})

target_compile_definitions(imgui
  PRIVATE
  IMGUI_DISABLE_DEFAULT_ALLOCATORS
  IMGUI_DISABLE_DEFAULT_FONT
  IMGUI_INCLUDE_IMGUI_USER_H
  IMGUI_USER_H_FILENAME="imgui_custom.h"
  IMGUI_DEFINE_MATH_OPERATORS
)

target_include_directories(imgui
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/imgui
)

target_sources_sandboxed(imgui
  DIRECTORY imgui

  STRIP
  ../IconFontCppHeaders-git/
  ../imgui-git/

  SOURCES
  imgui_custom.cpp
  imgui_custom.h

  imgui.natvis
  imgui.natstepfilter

  ../IconFontCppHeaders-git/IconsFontAwesome6.h
  
  ../imgui-git/imconfig.h
  ../imgui-git/imgui.cpp
  ../imgui-git/imgui.h
  ../imgui-git/imgui_demo.cpp
  ../imgui-git/imgui_draw.cpp
  ../imgui-git/imgui_internal.h
  ../imgui-git/imgui_tables.cpp
  ../imgui-git/imgui_widgets.cpp
  ../imgui-git/imstb_rectpack.h
  ../imgui-git/imstb_textedit.h
  ../imgui-git/imstb_truetype.h

  ../imgui-git/backends/imgui_impl_dx9.cpp
  ../imgui-git/backends/imgui_impl_dx9.h

  ../imgui-git/backends/imgui_impl_dx10.cpp
  ../imgui-git/backends/imgui_impl_dx10.h

  ../imgui-git/backends/imgui_impl_dx11.cpp
  ../imgui-git/backends/imgui_impl_dx11.h
)
