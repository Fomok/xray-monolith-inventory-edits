#pragma once

#include "IRenderDetailModel.h"

class ECORE_API CDetail : public IRender_DetailModel
{
public:

#if defined(USE_DX11)
    ref_geom hw_Geom;
    ID3DVertexBuffer* hw_VB;
    ID3DIndexBuffer* hw_IB;
#endif

	void Load(IReader* S);
	void Optimize();
	virtual void Unload();

	virtual void transfer(Fmatrix& mXform, fvfVertexOut* vDest, u32 C, u16* iDest, u32 iOffset);
	virtual void transfer(Fmatrix& mXform, fvfVertexOut* vDest, u32 C, u16* iDest, u32 iOffset, float du, float dv);
	virtual ~CDetail();
};
