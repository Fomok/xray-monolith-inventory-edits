#ifndef R4_JITTER_H
#define R4_JITTER_H

#include <blenders/Blender_Recorder.h>
#include <r2_types.h>

IC void jitter(CBlender_Compile& C)
{
	//	C.r_Sampler	("jitter0",	JITTER(0), true, D3DTADDRESS_WRAP, D3DTEXF_POINT, D3DTEXF_NONE, D3DTEXF_POINT);
	//	C.r_Sampler	("jitter1",	JITTER(1), true, D3DTADDRESS_WRAP, D3DTEXF_POINT, D3DTEXF_NONE, D3DTEXF_POINT);
	//	C.r_Sampler	("jitter2",	JITTER(2), true, D3DTADDRESS_WRAP, D3DTEXF_POINT, D3DTEXF_NONE, D3DTEXF_POINT);
	//	C.r_Sampler	("jitter3",	JITTER(3), true, D3DTADDRESS_WRAP, D3DTEXF_POINT, D3DTEXF_NONE, D3DTEXF_POINT);
	C.r_dx10Texture("jitter0", JITTER(0));
	C.r_dx10Texture("jitter1", JITTER(1));
	C.r_dx10Texture("jitter2", JITTER(2));
	C.r_dx10Texture("jitter3", JITTER(3));
	C.r_dx10Texture("jitter4", JITTER(4));
	C.r_dx10Texture("jitterMipped", r2_jitter_mipped);
	C.r_dx10Sampler("smp_jitter");
}

#endif // R4_JITTER_H