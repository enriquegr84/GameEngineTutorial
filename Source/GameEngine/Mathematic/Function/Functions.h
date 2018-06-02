// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "Mathematic/Arithmetic/Arithmetic.h"
#include "Constants.h"

#include <limits>

//template <int N, typename Real>
template <typename... Types>
class Function
{

};


// A specialization for 32-bit IEEE floating-point numbers.
template <>
class Function<float>
{
public:
	inline static float ACos(float x)
	{
		return acos(x);
	}

	inline static float ACosh(float x)
	{
		return log(x + sqrt(x * x - 1.0f));
	}

	inline static float ASin(float x)
	{
		return asin(x);
	}

	inline static float ASinh(float x)
	{
		return log(x + sqrt(x * x + 1.0f));
	}

	inline static float ATan(float x)
	{
		return atan(x);
	}

	inline static float ATanh(float x)
	{
		return log((1.0f + x) / (1.0f - x)) * 0.5f;
	}

	inline static float ATan2(float y, float x)
	{
		return atan2(y, x);
	}

	inline static float ATanpi(float x)
	{
		return atan(x) * (float)GE_C_INV_PI;
	}

	inline static float ATan2pi(float y, float x)
	{
		return atan2(y, x) * (float)GE_C_INV_PI;
	}

	inline static float Ceil(float x)
	{
		return ceil(x);
	}

	inline static float Cos(float x)
	{
		return cos(x);
	}

	inline static float Cosh(float x)
	{
		return cosh(x);
	}

	inline static float Cospi(float x)
	{
		return cos(x * (float)GE_C_PI);
	}

	inline static float Exp(float x)
	{
		return exp(x);
	}

	inline static float Exp2(float x)
	{
		return exp(x * (float)GE_C_LN_2);
	}

	inline static float Exp10(float x)
	{
		return exp(x * (float)GE_C_LN_10);
	}

	inline static float FAbs(float x)
	{
		return fabs(x);
	}

	inline static float Floor(float x)
	{
		return floor(x);
	}

	inline static float Fract(float x)
	{
		return x - floor(x);
	}

	inline static float FMod(float x, float y)
	{
		return fmod(x, y);
	}

	inline static float FRExp(float x, int* exponent)
	{
		return frexp(x, exponent);
	}

	inline static float InvSqrt(float x)
	{
		return 1.0f / sqrt(x);
	}

	inline static float LDExp(float x, int exponent)
	{
		return ldexp(x, exponent);
	}

	inline static float Log(float x)
	{
		return log(x);
	}

	inline static float Log2(float x)
	{
		return log(x) * (float)GE_C_INV_LN_2;
	}

	inline static float Log10(float x)
	{
		return log10(x);
	}

	inline static float Pow(float x, float y)
	{
		return pow(x, y);
	}

	inline static float Sin(float x)
	{
		return sin(x);
	}

	inline static float Sinh(float x)
	{
		return sinh(x);
	}

	inline static float Sinpi(float x)
	{
		return sin(x * (float)GE_C_PI);
	}

	inline static float Sqr(float x)
	{
		return x * x;
	}

	inline static float Sqrt(float x)
	{
		return sqrt(x);
	}

	inline static float Tan(float x)
	{
		return tan(x);
	}

	inline static float Tanh(float x)
	{
		return tanh(x);
	}

	inline static float Sign(float x)
	{
		return (x > 0.0f ? 1.0f : (x < 0.0f ? -1.0f : 0.0f));
	}

	inline static int ISign(float x)
	{
		return (x > 0.0f ? 1 : (x < 0.0f ? -1 : 0));
	}

	inline static float Clamp(float x, float min, float max)
	{
		return (x <= min ? min : (x >= max ? max : x));
	}

	inline static float Saturate(float x)
	{
		return (x <= 0.0f ? 0.0f : (x >= 1.0f ? 1.0f : x));
	}

	inline static bool Equals(float x, float y, float tolerance = GE_ROUNDING_ERROR)
	{
		return (x + tolerance >= y) && (x - tolerance <= y);
	}

	inline static bool IsNotZero(float x, float tolerance = GE_ROUNDING_ERROR)
	{
		return fabs(x) > tolerance;
	}

	inline static bool IsZero(float x, float tolerance = GE_ROUNDING_ERROR)
	{
		return fabs(x) <= tolerance;
	}

	inline static float Clamp(float const& x, float const& min, float const& max)
	{
		return (x <= min ? min : (x >= max ? max : x));
	}

	template <int N>
	inline static eastl::array<float, N> Lerp(eastl::array<float, N> v0, eastl::array<float, N> v1, float intp)
	{
		intp = eastl::clamp(intp, 0.f, 1.f);
		const float inv = 1.0f - intp;

		eastl::array<float, N> lerp = eastl::array<float, N>();
		for (int i = 0; i < N; i++)
			lerp[i] = round(v1[i] * inv + v0[i] * intp);

		return lerp;
	}

	template <int N>
	inline static Vector<N, float> Lerp(Vector<N, float> v0, Vector<N, float> v1, float intp)
	{
		intp = eastl::clamp(intp, 0.f, 1.f);
		const float inv = 1.0f - intp;

		Vector<N, float> lerp = Vector<N, float>();
		for (int i = 0; i < N; i++)
			lerp[i] = round(v1[i] * inv + v0[i] * intp);

		return lerp;
	}

	inline static void ChebyshevRatios(float t, float cosA, float& f0, float& f1)
	{
		if (cosA < 1.0f)
		{
			// The angle A is in (0,pi/2].
			float A = acos(cosA);
			float invSinA = 1.0f / sin(A);
			f0 = sin((1.0f - t) * A) * invSinA;
			f1 = sin(t * A) * invSinA;
		}
		else
		{
			// The angle theta is 0.
			f0 = 1.0f - t;
			f1 = t;
		}
	}

	inline static unsigned int GetMaxBisections()
	{
		return (unsigned int)(3 + eastl::numeric_limits<float>::digits - eastl::numeric_limits<float>::min_exponent);
	}
};

// A specialization for 64-bit IEEE floating-point numbers.
template <>
class Function<double>
{
public:
	inline static double ACos(double x)
	{
		return acos(x);
	}

	inline static double ACosh(double x)
	{
		return log(x + sqrt(x * x - 1.0));
	}

	inline static double ASin(double x)
	{
		return asin(x);
	}

	inline static double ASinh(double x)
	{
		return log(x + sqrt(x * x + 1.0));
	}

	inline static double ATan(double x)
	{
		return atan(x);
	}

	inline static double ATanh(double x)
	{
		return log((1.0 + x) / (1.0 - x)) * 0.5;
	}

	inline static double ATan2(double y, double x)
	{
		return atan2(y, x);
	}

	inline static double ATanpi(double x)
	{
		return atan(x) * GE_C_INV_PI;
	}

	inline static double ATan2pi(double y, double x)
	{
		return atan2(y, x) * GE_C_INV_PI;
	}

	inline static double Ceil(double x)
	{
		return ceil(x);
	}

	inline static double Cos(double x)
	{
		return cos(x);
	}

	inline static double Cosh(double x)
	{
		return cosh(x);
	}

	inline static double Cospi(double x)
	{
		return cos(x * GE_C_PI);
	}

	inline static double Exp(double x)
	{
		return exp(x);
	}

	inline static double Exp2(double x)
	{
		return exp(x * GE_C_LN_2);
	}

	inline static double Exp10(double x)
	{
		return exp(x * GE_C_LN_10);
	}

	inline static double FAbs(double x)
	{
		return fabs(x);
	}

	inline static double Floor(double x)
	{
		return floor(x);
	}

	inline static double Fract(double x)
	{
		return x - floor(x);
	}

	inline static double FMod(double x, double y)
	{
		return fmod(x, y);
	}

	inline static double FRExp(double x, int* exponent)
	{
		return frexp(x, exponent);
	}

	inline static double InvSqrt(double x)
	{
		return 1.0 / sqrt(x);
	}

	inline static double LDExp(double x, int exponent)
	{
		return ldexp(x, exponent);
	}

	inline static double Log(double x)
	{
		return log(x);
	}

	inline static double Log2(double x)
	{
		return log(x) * GE_C_INV_LN_2;
	}

	inline static double Log10(double x)
	{
		return log10(x);
	}

	inline static double Pow(double x, double y)
	{
		return pow(x, y);
	}

	inline static double Sin(double x)
	{
		return sin(x);
	}

	inline static double Sinh(double x)
	{
		return sinh(x);
	}

	inline static double Sinpi(double x)
	{
		return sin(x * GE_C_PI);
	}

	inline static double Sqr(double x)
	{
		return x * x;
	}

	inline static double Sqrt(double x)
	{
		return sqrt(x);
	}

	inline static double Tan(double x)
	{
		return tan(x);
	}

	inline static double Tanh(double x)
	{
		return tanh(x);
	}

	inline static double Sign(double x)
	{
		return (x > 0.0 ? 1.0 : (x < 0.0 ? -1.0 : 0.0));
	}

	inline static int ISign(double x)
	{
		return (x > 0.0 ? 1 : (x < 0.0 ? -1 : 0));
	}

	inline static double Clamp(double x, double min, double max)
	{
		return (x <= min ? min : (x >= max ? max : x));
	}

	inline static double Saturate(double x)
	{
		return (x <= 0.0 ? 0.0 : (x >= 1.0 ? 1.0 : x));
	}

	inline static bool Equals(double x, double y, double tolerance = GE_ROUNDING_ERROR)
	{
		return (x + tolerance >= y) && (x - tolerance <= y);
	}

	inline static bool IsNotZero(double x, double tolerance = GE_ROUNDING_ERROR)
	{
		return fabs(x) > tolerance;
	}

	inline static bool IsZero(double x, double tolerance = GE_ROUNDING_ERROR)
	{
		return fabs(x) <= tolerance;
	}

	inline static double Clamp(double const& x, double const& min, double const& max)
	{
		return (x <= min ? min : (x >= max ? max : x));
	}

	template <int N>
	inline static eastl::array<double, N> Lerp(eastl::array<double, N> v0, eastl::array<double, N> v1, double intp)
	{
		intp = clamp(intp, 0.f, 1.f);
		const double inv = 1.0f - intp;

		eastl::array<double, N> lerp = eastl::array<double, N>();
		for (int i = 0; i < N; i++)
			lerp[i] = round(v1[i] * inv + v0[i] * intp);

		return lerp;
	}

	template <int N>
	inline static Vector<N, double> Lerp(Vector<N, double> v0, Vector<N, double> v1, double intp)
	{
		intp = clamp(intp, 0.f, 1.f);
		const double inv = 1.0f - intp;

		Vector<N, double> lerp = Vector<N, double>();
		for (int i = 0; i < N; i++)
			lerp[i] = round(v1[i] * inv + v0[i] * intp);

		return lerp;
	}

	inline static unsigned int GetMaxBisections()
	{
		return (unsigned int)(3 + eastl::numeric_limits<double>::digits - eastl::numeric_limits<double>::min_exponent);
	}
};

#endif