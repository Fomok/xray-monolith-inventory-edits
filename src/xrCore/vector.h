#ifndef _vector_included
#define _vector_included

ICF int iFloor(float x);
ICF int iCeil(float x);

#include "math_constants.h"

// Define types and namespaces (CPU & FPU)
#include "_types.h"
#include "_math.h"
#include "_bitwise.h"
#include "_std_extensions.h"
#include "math_funcs_inline.h"

// linear interpolation
template <class T>
inline constexpr T _lerp(const T& _val_a, const T& _val_b, const float& _factor)
{
	return (_val_a * (1.0f - _factor)) + (_val_b * _factor);
}

template <class T>
inline constexpr T _lerpc(const T& _val_a, const T& _val_b, const float& _factor)
{
	float factor_c = clampr(_factor, 0.0f, 1.0f);
	return (_val_a * (1.0 - factor_c)) + (_val_b * factor_c);
}

// inertion
IC float _inertion(float _val_cur, float _val_trgt, float _friction)
{
	float friction_i = 1.f - _friction;
	return _val_cur * _friction + _val_trgt * friction_i;
}

// pre-definitions
template <class T>
struct _quaternion;

#pragma pack(push)
#pragma pack(1)

#include "_random.h"

#include "_color.h"
#include "_vector3d.h"
#include "_vector2.h"
#include "_vector4.h"
#include "_matrix.h"
#include "_matrix33.h"
#include "_quaternion.h"
#include "_rect.h"
#include "_fbox.h"
#include "_fbox2.h"
#include "_obb.h"
#include "_sphere.h"
#include "_cylinder.h"
#include "_random.h"
#include "_compressed_normal.h"
#include "_plane.h"
#include "_plane2.h"
#include "_flags.h"
#include "_matrix_ext.h"
#include "_quaternion_ext.h"
#include "math_funcs.h"
#ifdef DEBUG
#include "dump_string.h"
#endif
#pragma pack(pop)

#endif
