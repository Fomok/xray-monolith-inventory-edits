add_custom_target(dxsdk)

set_property(
  TARGET dxsdk
  PROPERTY FOLDER
  ${FOLDER_SDK}
)

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
