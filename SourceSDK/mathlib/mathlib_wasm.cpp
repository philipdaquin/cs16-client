//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: WebAssembly-safe mathlib shim.
//
// This keeps the SourceSDK math function-pointer contract alive without
// pulling in the desktop SSE/3DNow implementation that Emscripten rejects.
//
//===========================================================================//

#include <math.h>
#include <float.h>

#include "mathlib/mathlib.h"

bool s_bMathlibInitialized = true;

static bool s_b3DNowEnabled = false;
static bool s_bMMXEnabled = false;
static bool s_bSSEEnabled = false;
static bool s_bSSE2Enabled = false;

float _sqrtf(float x)
{
	return sqrtf(x);
}

float _rsqrtf(float x)
{
	return 1.0f / sqrtf(x);
}

float FASTCALL _VectorNormalize(Vector& vec)
{
	const float radius = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	const float iradius = 1.0f / (radius + FLT_EPSILON);

	vec.x *= iradius;
	vec.y *= iradius;
	vec.z *= iradius;

	return radius;
}

void FASTCALL _VectorNormalizeFast(Vector& vec)
{
	const float iradius = 1.0f / (sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z) + FLT_EPSILON);

	vec.x *= iradius;
	vec.y *= iradius;
	vec.z *= iradius;
}

float _InvRSquared(const float* v)
{
	const float r2 = DotProduct(v, v);
	return r2 < 1.0f ? 1.0f : 1.0f / r2;
}

float (*pfSqrt)(float x) = _sqrtf;
float (*pfRSqrt)(float x) = _rsqrtf;
float (*pfRSqrtFast)(float x) = _rsqrtf;
float (FASTCALL *pfVectorNormalize)(Vector& v) = _VectorNormalize;
void (FASTCALL *pfVectorNormalizeFast)(Vector& v) = _VectorNormalizeFast;
float (*pfInvRSquared)(const float* v) = _InvRSquared;
void (*pfFastSinCos)(float x, float* s, float* c) = SinCos;
float (*pfFastCos)(float x) = cosf;

float SinCosTable[SIN_TABLE_SIZE];

void InitSinCosTable()
{
	for (int i = 0; i < SIN_TABLE_SIZE; ++i)
	{
		SinCosTable[i] = sin(i * 2.0 * M_PI / SIN_TABLE_SIZE);
	}
}

qboolean VectorsEqual(const float* v1, const float* v2)
{
	return ((v1[0] == v2[0]) && (v1[1] == v2[1]) && (v1[2] == v2[2]));
}

void MatrixVectors(const matrix3x4_t& matrix, Vector* pForward, Vector* pRight, Vector* pUp)
{
	MatrixGetColumn(matrix, 0, *pForward);
	MatrixGetColumn(matrix, 1, *pRight);
	MatrixGetColumn(matrix, 2, *pUp);
	*pRight *= -1.0f;
}

void VectorVectors(const Vector& forward, Vector& right, Vector& up)
{
	Vector tmp;

	if (forward[0] == 0 && forward[1] == 0)
	{
		right[0] = 0;
		right[1] = -1;
		right[2] = 0;
		up[0] = -forward[2];
		up[1] = 0;
		up[2] = 0;
	}
	else
	{
		tmp[0] = 0;
		tmp[1] = 0;
		tmp[2] = 1.0f;
		CrossProduct(forward, tmp, right);
		VectorNormalize(right);
		CrossProduct(right, forward, up);
		VectorNormalize(up);
	}
}

void MathLib_Init(float gamma, float texGamma, float brightness, int overbright, bool bAllow3DNow, bool bAllowSSE, bool bAllowSSE2, bool bAllowMMX)
{
	(void)gamma;
	(void)texGamma;
	(void)brightness;
	(void)overbright;
	(void)bAllow3DNow;
	(void)bAllowSSE;
	(void)bAllowSSE2;
	(void)bAllowMMX;

	s_bMathlibInitialized = true;
	s_b3DNowEnabled = false;
	s_bMMXEnabled = false;
	s_bSSEEnabled = false;
	s_bSSE2Enabled = false;

	pfSqrt = _sqrtf;
	pfRSqrt = _rsqrtf;
	pfRSqrtFast = _rsqrtf;
	pfVectorNormalize = _VectorNormalize;
	pfVectorNormalizeFast = _VectorNormalizeFast;
	pfInvRSquared = _InvRSquared;
	pfFastSinCos = SinCos;
	pfFastCos = cosf;

	InitSinCosTable();
	BuildGammaTable(gamma, texGamma, brightness, overbright);
}

bool MathLib_3DNowEnabled(void)
{
	return s_b3DNowEnabled;
}

bool MathLib_MMXEnabled(void)
{
	return s_bMMXEnabled;
}

bool MathLib_SSEEnabled(void)
{
	return s_bSSEEnabled;
}

bool MathLib_SSE2Enabled(void)
{
	return s_bSSE2Enabled;
}

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
