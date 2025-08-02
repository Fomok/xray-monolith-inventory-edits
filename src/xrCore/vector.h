#ifndef _vector_included
#define _vector_included

ICF int iFloor(float x);
ICF int iCeil(float x);

// Undef some macros
#ifdef M_PI
#undef M_PI
#endif

#ifdef PI
#undef PI
#endif

// Select platform
#ifdef _MSC_VER
#define M_VISUAL
#endif

// Constants
const float EPS_S = 0.0000001f;
const float EPS = 0.0000100f;
const float EPS_L = 0.0010000f;

#undef M_SQRT1_2
const float M_SQRT1_2 = 0.7071067811865475244008443621048f; //490;

const float M_PI = 3.1415926535897932384626433832795f;
const float PI = 3.1415926535897932384626433832795f;
const float PI_MUL_2 = 6.2831853071795864769252867665590f;
const float PI_MUL_3 = 9.4247779607693797153879301498385f;
const float PI_MUL_4 = 12.566370614359172953850573533118f;
const float PI_MUL_6 = 18.849555921538759430775860299677f;
const float PI_MUL_8 = 25.132741228718345907701147066236f;
const float PI_DIV_2 = 1.5707963267948966192313216916398f;
const float PI_DIV_3 = 1.0471975511965977461542144610932f;
const float PI_DIV_4 = 0.7853981633974483096156608458199f;
const float PI_DIV_6 = 0.5235987755982988730771072305466f;
const float PI_DIV_8 = 0.3926990816987241548078304229099f;

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
#include "math_funcs.h"
#ifdef DEBUG
#include "dump_string.h"
#endif
#pragma pack(pop)

template <class T>
IC _matrix<T>& _matrix<T>::rotation(const _quaternion<T>& Q)
{
	T xx = Q.x * Q.x;
	T yy = Q.y * Q.y;
	T zz = Q.z * Q.z;
	T xy = Q.x * Q.y;
	T xz = Q.x * Q.z;
	T yz = Q.y * Q.z;
	T wx = Q.w * Q.x;
	T wy = Q.w * Q.y;
	T wz = Q.w * Q.z;

	_11 = 1 - 2 * (yy + zz);
	_12 = 2 * (xy - wz);
	_13 = 2 * (xz + wy);
	_14 = 0;
	_21 = 2 * (xy + wz);
	_22 = 1 - 2 * (xx + zz);
	_23 = 2 * (yz - wx);
	_24 = 0;
	_31 = 2 * (xz - wy);
	_32 = 2 * (yz + wx);
	_33 = 1 - 2 * (xx + yy);
	_34 = 0;
	_41 = 0;
	_42 = 0;
	_43 = 0;
	_44 = 1;
	return *this;
}

template <class T>
IC _matrix<T>& _matrix<T>::mk_xform(const _quaternion<T>& Q, const Tvector& V)
{
	T xx = Q.x * Q.x;
	T yy = Q.y * Q.y;
	T zz = Q.z * Q.z;
	T xy = Q.x * Q.y;
	T xz = Q.x * Q.z;
	T yz = Q.y * Q.z;
	T wx = Q.w * Q.x;
	T wy = Q.w * Q.y;
	T wz = Q.w * Q.z;

	_11 = 1 - 2 * (yy + zz);
	_12 = 2 * (xy - wz);
	_13 = 2 * (xz + wy);
	_14 = 0;
	_21 = 2 * (xy + wz);
	_22 = 1 - 2 * (xx + zz);
	_23 = 2 * (yz - wx);
	_24 = 0;
	_31 = 2 * (xz - wy);
	_32 = 2 * (yz + wx);
	_33 = 1 - 2 * (xx + yy);
	_34 = 0;
	_41 = V.x;
	_42 = V.y;
	_43 = V.z;
	_44 = 1;
	return *this;
}

#define TRACE_QZERO_TOLERANCE 0.1f

template <class T>
IC _quaternion<T>& _quaternion<T>::set(const _matrix<T>& M)
{
	float trace, s;

	trace = M._11 + M._22 + M._33;
	if (trace > 0.0f)
	{
		s = _sqrt(trace + 1.0f);
		w = s * 0.5f;
		s = 0.5f / s;

		x = (M._32 - M._23) * s;
		y = (M._13 - M._31) * s;
		z = (M._21 - M._12) * s;
	}
	else
	{
		int biggest;
		enum { A, E, I };
		if (M._11 > M._22)
		{
			if (M._33 > M._11)
				biggest = I;
			else
				biggest = A;
		}
		else
		{
			if (M._33 > M._11)
				biggest = I;
			else
				biggest = E;
		}

		// in the unusual case the original trace fails to produce a good sqrt, try others...
		switch (biggest)
		{
		case A:
			s = _sqrt(M._11 - (M._22 + M._33) + 1.0f);
			if (s > TRACE_QZERO_TOLERANCE)
			{
				x = s * 0.5f;
				s = 0.5f / s;
				w = (M._32 - M._23) * s;
				y = (M._12 + M._21) * s;
				z = (M._13 + M._31) * s;
				break;
			}
			// I
			s = _sqrt(M._33 - (M._11 + M._22) + 1.0f);
			if (s > TRACE_QZERO_TOLERANCE)
			{
				z = s * 0.5f;
				s = 0.5f / s;
				w = (M._21 - M._12) * s;
				x = (M._31 + M._13) * s;
				y = (M._32 + M._23) * s;
				break;
			}
			// E
			s = _sqrt(M._22 - (M._33 + M._11) + 1.0f);
			if (s > TRACE_QZERO_TOLERANCE)
			{
				y = s * 0.5f;
				s = 0.5f / s;
				w = (M._13 - M._31) * s;
				z = (M._23 + M._32) * s;
				x = (M._21 + M._12) * s;
				break;
			}
			break;
		case E:
			s = _sqrt(M._22 - (M._33 + M._11) + 1.0f);
			if (s > TRACE_QZERO_TOLERANCE)
			{
				y = s * 0.5f;
				s = 0.5f / s;
				w = (M._13 - M._31) * s;
				z = (M._23 + M._32) * s;
				x = (M._21 + M._12) * s;
				break;
			}
			// I
			s = _sqrt(M._33 - (M._11 + M._22) + 1.0f);
			if (s > TRACE_QZERO_TOLERANCE)
			{
				z = s * 0.5f;
				s = 0.5f / s;
				w = (M._21 - M._12) * s;
				x = (M._31 + M._13) * s;
				y = (M._32 + M._23) * s;
				break;
			}
			// A
			s = _sqrt(M._11 - (M._22 + M._33) + 1.0f);
			if (s > TRACE_QZERO_TOLERANCE)
			{
				x = s * 0.5f;
				s = 0.5f / s;
				w = (M._32 - M._23) * s;
				y = (M._12 + M._21) * s;
				z = (M._13 + M._31) * s;
				break;
			}
			break;
		case I:
			s = _sqrt(M._33 - (M._11 + M._22) + 1.0f);
			if (s > TRACE_QZERO_TOLERANCE)
			{
				z = s * 0.5f;
				s = 0.5f / s;
				w = (M._21 - M._12) * s;
				x = (M._31 + M._13) * s;
				y = (M._32 + M._23) * s;
				break;
			}
			// A
			s = _sqrt(M._11 - (M._22 + M._33) + 1.0f);
			if (s > TRACE_QZERO_TOLERANCE)
			{
				x = s * 0.5f;
				s = 0.5f / s;
				w = (M._32 - M._23) * s;
				y = (M._12 + M._21) * s;
				z = (M._13 + M._31) * s;
				break;
			}
			// E
			s = _sqrt(M._22 - (M._33 + M._11) + 1.0f);
			if (s > TRACE_QZERO_TOLERANCE)
			{
				y = s * 0.5f;
				s = 0.5f / s;
				w = (M._13 - M._31) * s;
				z = (M._23 + M._32) * s;
				x = (M._21 + M._12) * s;
				break;
			}
			break;
		}
	}
	return *this;
}

#endif
