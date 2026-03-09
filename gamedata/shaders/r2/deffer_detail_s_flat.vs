#include "common.h"

uniform float4 		consts; // {1/quant,1/quant,diffusescale,ambient}
uniform float4 		float2x4 array[50] : register(c12);

float3x3 setMatrix (float3 hpb)
{		
	float _ch, _cp, _cb, _sh, _sp, _sb, _cc, _cs, _sc, _ss;

	sincos(hpb.x, _sh, _ch);
	sincos(hpb.y, _sp, _cp);
	sincos(hpb.z, _sb, _cb);
	
	_cc = _ch*_cb; _cs = _ch*_sb; _sc = _sh*_cb; _ss = _sh*_sb;

	return float3x3(_cc-_sp*_ss, _sp*_sc+_cs, -_cp*_sh,
				-_cp*_sb, 	 _cp*_cb,	  _sp,
				_sp*_cs+_sc, _ss-_sp*_cc, _cp*_ch);
};

p_flat 	main (v_detail v)
{
	p_flat 		O;
	// index
	int 	i 	= v.misc.w;
	float2x4 mm = array[i];

	float3x3 mmhpb = setMatrix(mm[0].xyz);
	float3 posi = float3(mm[1].xyz);

	float scale = mm[0].w;
	float hemi = abs(mm[1].w);
	float sun = sign(mm[1].w)*0.25f+0.25f;
	float4 m0 = float4(mmhpb[0]*scale, posi.x);
	float4 m1 = float4(mmhpb[1]*scale, posi.y);
	float4 m2 = float4(mmhpb[2]*scale, posi.z);

	// Transform pos to world coords
	float4 	pos;
 	pos.x 		= dot	(m0, v.pos);
 	pos.y 		= dot	(m1, v.pos);
 	pos.z 		= dot	(m2, v.pos);
	pos.w 		= 1;

	// Normal in world coords
	float3 	norm;	
		norm.x 	= pos.x - m0.w	;
		norm.y 	= pos.y - m1.w	+ .75f;	// avoid zero
		norm.z	= pos.z - m2.w	;

	// Final out
	float4	Pp 	= mul		(m_WVP,	pos				);
	O.hpos 		= Pp;
	O.N 		= mul		(m_WV,  normalize(norm)	);
	float3	Pe	= mul		(m_WV,  pos				);
	O.tcdh 		= float4	((v.misc * consts).xyyy	);

# if defined(USE_R2_STATIC_SUN)
	O.tcdh.w	= sun;								// (,,,dir-occlusion)
# endif

	O.position	= float4	(Pe, 		hemi		);

	return O;
}
FXVS;
