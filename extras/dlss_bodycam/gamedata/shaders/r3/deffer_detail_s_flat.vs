#ifdef DETAILS_INSTANCING
#include "common.h"

#include "screenspace_mvectors.h"

uniform float4 		consts; // {1/quant,1/quant,diffusescale,ambient}

#include "dt_instance_decode.h"

v2p_flat 	main (v_detail v, uint iid : SV_InstanceID)
{
	v2p_flat 		O;

	dt_instance rec = dt_instances[(uint)dt_instance_base.x + iid];

	// rotation rides three snorm16 components, the dropped one is positive
	int3 qi = int3(asint(rec.a.w) << 16, asint(rec.a.w), asint(rec.b.x) << 16) >> 16;
	float3 qv = qi * (0.70710678f / 32767.0f);
	float4 q = float4(qv, sqrt(max(0.0f, 1.0f - dot(qv, qv))));
	uint qd = rec.b.x >> 16;
	q = qd == 0 ? q.wxyz : (qd == 1 ? q.xwyz : (qd == 2 ? q.xywz : q));

	float3 q2 = q.xyz + q.xyz;
	float3 qq = q.xyz * q2;			// 2xx 2yy 2zz
	float3 qc = q.xyz * q2.yzx;		// 2xy 2yz 2zx
	float3 qw = q.w * q2;			// 2wx 2wy 2wz
	float  sc = asfloat(rec.b.w);

	float4  m0 	= float4(float3(1.0f - (qq.y + qq.z), qc.x - qw.z, qc.z + qw.y) * sc, asfloat(rec.a.x));
	float4  m1 	= float4(float3(qc.x + qw.z, 1.0f - (qq.x + qq.z), qc.y - qw.x) * sc, asfloat(rec.a.y));
	float4  m2 	= float4(float3(qc.z - qw.y, qc.y + qw.x, 1.0f - (qq.x + qq.y)) * sc, asfloat(rec.a.z));

	float4 inst_sh = float4(((rec.b.z >> 16) & 0xFF) * (1.0f / 255.0f), (rec.b.z >> 24) * (1.0f / 255.0f), 0, 0);

	// Transform pos to world coords
	float4 	pos;
 	pos.x 		= dot	(m0, v.pos);
 	pos.y 		= dot	(m1, v.pos);
 	pos.z 		= dot	(m2, v.pos);
	pos.w 		= 1;

	// Per phase fade scales the blade around its pivot
	float3 Tr = float3(m0.w, m1.w, m2.w);
	float fade = 1.0f;
	if (dt_fade_params.x > 2.5f)
		fade = dt_fade_params.y; // the engine's per draw scale
	else if (dt_fade_params.x > 1.5f)
	{
		float2 dxz = Tr.xz - dt_fade_params.zw;
		fade = 1.0f - dot(dxz, dxz) * 0.005f;
	}
	else if (dt_fade_params.x > 0.5f)
	{
		float3 dc = Tr - eye_position;
		fade = 1.0f - max(dot(dc, dc) - dt_fade_params.y, 0.0f) * 0.005f;
	}
	pos.xyz = Tr + (pos.xyz - Tr) * saturate(fade);

	// Normal in world coords
	float3 	norm;
		norm.x 	= pos.x - m0.w	;
		norm.y 	= pos.y - m1.w	+ .75f;	// avoid zero
		norm.z	= pos.z - m2.w	;

	// Final out
	float4	Pp 	= mul		(m_WVP,	pos				);
	O.hpos 		= Pp;
	float3 orig		= mul		(m_WV,  normalize(norm)	);

	// TAA Jitter
	O.hpos.xy = ssfx_taa_jitter(O.hpos);

O.N = lerp(orig, mul((float3x3)m_WV,  v.pos), 0.25);
	float3	Pe	= mul		(m_WV,  pos				);
	O.tcdh 		= float4	((v.misc * consts).xyyy	);

# if defined(USE_R2_STATIC_SUN)
	O.tcdh.w	= inst_sh.x;						// (,,,dir-occlusion)
# endif

	O.position	= float4	(Pe, 		inst_sh.y	);

	return O;
}
FXVS;
#else
#include "common.h"

#include "screenspace_mvectors.h"

uniform float4 		consts; // {1/quant,1/quant,diffusescale,ambient}
//uniform float4 		array	[200] : register(c12);
//tbuffer DetailsData
//{
	uniform float4 		array[61*4];
//}

v2p_flat 	main (v_detail v)
{
	v2p_flat 		O;
	// index
	int 	i 	= v.misc.w;
	float4  m0 	= array[i+0];
	float4  m1 	= array[i+1];
	float4  m2 	= array[i+2];
	float4  c0 	= array[i+3];

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
	float3 orig		= mul		(m_WV,  normalize(norm)	);

	// TAA Jitter
	O.hpos.xy = ssfx_taa_jitter(O.hpos); 

O.N = lerp(orig, mul((float3x3)m_WV,  v.pos), 0.25);
	float3	Pe	= mul		(m_WV,  pos				);
	O.tcdh 		= float4	((v.misc * consts).xyyy	);

# if defined(USE_R2_STATIC_SUN)
	O.tcdh.w	= c0.x;								// (,,,dir-occlusion)
# endif

	O.position	= float4	(Pe, 		c0.w		);

	return O;
}
FXVS;
#endif
