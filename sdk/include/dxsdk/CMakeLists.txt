add_library(dxsdk INTERFACE)
target_folder(dxsdk ${FOLDER_SDK})

target_sources(dxsdk
  PRIVATE
  D3DX10.h
  d3dx10async.h
  D3DX10core.h
  D3DX10math.h
  D3DX10mesh.h
  D3DX10tex.h
  D3DX11.h
  D3DX11async.h
  D3DX11core.h
  D3DX11tex.h
  d3dx9.h
  d3dx9anim.h
  d3dx9core.h
  d3dx9effect.h
  d3dx9math.h
  d3dx9math.inl
  d3dx9mesh.h
  d3dx9shader.h
  d3dx9shape.h
  d3dx9tex.h
  d3dx9xof.h
  dxerr.h
)

target_include_directories(dxsdk
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(dxsdk
  INTERFACE
  $<$<CXX_COMPILER_ID:Clang>:-Wno-new-returns-null>
)

find_library(LIBRARY_D3D9 d3d9 HINTS ${XRAY_SDK_LIB})
find_library(LIBRARY_D3DX9 d3dx9 HINTS ${XRAY_SDK_LIB})
find_library(LIBRARY_D3D10 d3d10 HINTS ${XRAY_SDK_LIB})
find_library(LIBRARY_D3DX10 d3dx10 HINTS ${XRAY_SDK_LIB})
find_library(LIBRARY_D3D11 d3d11 HINTS ${XRAY_SDK_LIB})
find_library(LIBRARY_D3DX11 d3dx11 HINTS ${XRAY_SDK_LIB})

target_link_libraries(dxsdk
  INTERFACE
  ${LIBRARY_D3D9}
  ${LIBRARY_D3DX9}
  ${LIBRARY_D3D10}
  ${LIBRARY_D3DX10}
  ${LIBRARY_D3D11}
  ${LIBRARY_D3DX11}
)
