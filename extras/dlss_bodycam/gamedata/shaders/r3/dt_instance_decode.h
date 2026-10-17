#ifdef DETAILS_INSTANCING
// shared detail instance record and buffer bindings for the instanced detail shaders

// one 32 byte record per blade, indexed by SV_InstanceID
struct dt_instance
{
	uint4 a;	// pos xyz | rotation xy
	uint4 b;	// rotation z and dropped index | normal oct | alpha sun hemi | scale
};
StructuredBuffer<dt_instance> dt_instances : register(t3);
float4 dt_instance_base;	// x = first record of this draw's range
float4 dt_fade_params;	// x mode: 3 constant scale y, 2 light xz zw, 1 camera dist2 threshold y
#endif
