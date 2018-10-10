// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef MATRIX2_H
#define MATRIX2_H

#include "Mathematic/MathematicStd.h"

#include "Matrix.h"
#include "Vector2.h"

// Template alias for convenience.
template <typename Real>
using Matrix2x2 = Matrix<2, 2, Real>;

// Create a rotation matrix from an angle (in radians).  The matrix is
// [GE_USE_MAT_VEC]
//   R(t) = {{c,-s},{s,c}}
// [GE_USE_VEC_MAT]
//   R(t) = {{c,s},{-s,c}}
// where c = cos(t), s = sin(t), and the inner-brace pairs are rows of the
// matrix.
template <typename Real>
void MakeRotation(Real angle, Matrix2x2<Real>& rotation);

// Get the angle (radians) from a rotation matrix.  The caller is
// responsible for ensuring the matrix is a rotation.
template <typename Real>
Real GetRotationAngle(Matrix2x2<Real> const& rotation);

// Geometric operations.
template <typename Real>
Matrix2x2<Real> Inverse(Matrix2x2<Real> const& M,
    bool* reportInvertibility = nullptr);

template <typename Real>
Matrix2x2<Real> Adjoint(Matrix2x2<Real> const& M);

template <typename Real>
Real Determinant(Matrix2x2<Real> const& M);

template <typename Real>
Real Trace(Matrix2x2<Real> const& M);


template <typename Real>
void MakeRotation(Real angle, Matrix2x2<Real>& rotation)
{
	Real* rot = reinterpret_cast<Real*>(&rotation);

    Real cs = cos(angle);
    Real sn = sin(angle);
#if defined(GE_USE_MAT_VEC)
	rot[0 * 2 + 0] = cs;
	rot[0 * 2 + 1] = -sn;
	rot[1 * 2 + 0] = sn;
	rot[1 * 2 + 1] = cs;
#else
	rot[0 * 2 + 0] = cs;
	rot[0 * 2 + 1] = sn;
	rot[1 * 2 + 0] = -sn;
	rot[1 * 2 + 1] = cs;
#endif
}

template <typename Real>
Real GetRotationAngle(Matrix2x2<Real> const& rotation)
{
	Real const* rot = reinterpret_cast<Real const*>(&rotation);

#if defined(GE_USE_MAT_VEC)
    return atan2(rot[1 * 2 + 0], rot[0 * 2 + 0]);
#else
    return atan2(rot[0 * 2 + 1], rot[0 * 2 + 0]);
#endif
}

template <typename Real>
Matrix2x2<Real> Inverse(Matrix2x2<Real> const& M, bool* reportInvertibility)
{
	Real const* m = reinterpret_cast<Real const*>(&M);

    Matrix2x2<Real> inverse;
    bool invertible;
    Real det = m[0 * 2 + 0]* M[1 * 2 + 1] - m[0 * 2 + 1]* m[1 * 2 + 0];
    if (det != (Real)0)
    {
        Real invDet = ((Real)1) / det;
        inverse = Matrix2x2<Real>
        {
			m[1 * 2 + 1]*invDet, -m[0 * 2 + 1]*invDet,
			-m[1 * 2 + 0]*invDet, m[0 * 2 + 0]*invDet
        };
        invertible = true;
    }
    else
    {
        inverse.MakeZero();
        invertible = false;
    }

    if (reportInvertibility)
    {
        *reportInvertibility = invertible;
    }
    return inverse;
}

template <typename Real>
Matrix2x2<Real> Adjoint(Matrix2x2<Real> const& M)
{
	Real const* m = reinterpret_cast<Real const*>(&M);

    return Matrix2x2<Real>
    {
		m[1 * 2 + 1], -m[0 * 2 + 1],
		-m[1 * 2 + 0], m[0 * 2 + 0]
    };
}

template <typename Real>
Real Determinant(Matrix2x2<Real> const& M)
{
	Real const* m = reinterpret_cast<Real const*>(&M);

    Real det = m[0 * 2 + 0]* m[1 * 2 + 1] - m[0 * 2 + 1]* m[1 * 2 + 0];
    return det;
}

template <typename Real>
Real Trace(Matrix2x2<Real> const& M)
{
	Real const* m = reinterpret_cast<Real const*>(&M);

    Real trace = m[0 * 2 + 0] + m[1 * 2 + 1];
    return trace;
}


#endif