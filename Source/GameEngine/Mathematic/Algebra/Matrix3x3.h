// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef MATRIX3_H
#define MATRIX3_H

#include "Matrix.h"
#include "Vector3.h"

// Template alias for convenience.
template <typename Real>
using Matrix3x3 = Matrix<3, 3, Real>;

// Geometric operations.
template <typename Real>
Matrix3x3<Real> Inverse(Matrix3x3<Real> const& M,
    bool* reportInvertibility = nullptr);

template <typename Real>
Matrix3x3<Real> Adjoint(Matrix3x3<Real> const& M);

template <typename Real>
Real Determinant(Matrix3x3<Real> const& M);

template <typename Real>
Real Trace(Matrix3x3<Real> const& M);


template <typename Real>
Matrix3x3<Real> Inverse(Matrix3x3<Real> const& M, bool* reportInvertibility)
{
	Real const* m = reinterpret_cast<Real const*>(&M);

    Matrix3x3<Real> inverse;
    bool invertible;
    Real c00 = m[1 * 3 + 1]*m[2 * 3 + 2] - m[1 * 3 + 2]*m[2 * 3 + 1];
    Real c10 = m[1 * 3 + 2]*m[2 * 3 + 0] - m[1 * 3 + 0]*m[2 * 3 + 2];
    Real c20 = m[1 * 3 + 0]*m[2 * 3 + 1] - m[1 * 3 + 1]*m[2 * 3 + 0];
    Real det = m[0 * 3 + 0]*c00 + m[0 * 3 + 1]*c10 + m[0 * 3 + 2]*c20;
    if (det != (Real)0)
    {
        Real invDet = ((Real)1) / det;
        inverse = Matrix3x3<Real>
        {
            c00*invDet,
			(m[0 * 3 + 2]*m[2 * 3 + 1] - m[0 * 3 + 1]*m[2 * 3 + 2])*invDet,
			(m[0 * 3 + 1]*m[1 * 3 + 2] - m[0 * 3 + 2]*m[1 * 3 + 1])*invDet,
			c10*invDet,
			(m[0 * 3 + 0]*m[2 * 3 + 2] - m[0 * 3 + 2]*m[2 * 3 + 0])*invDet,
			(m[0 * 3 + 2]*m[1 * 3 + 0] - m[0 * 3 + 0]*m[1 * 3 + 2])*invDet,
			c20*invDet,
			(m[0 * 3 + 1]*m[2 * 3 + 0] - m[0 * 3 + 0]*m[2 * 3 + 1])*invDet,
			(m[0 * 3 + 0]*m[1 * 3 + 1] - m[0 * 3 + 1]*m[1 * 3 + 0])*invDet
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
Matrix3x3<Real> Adjoint(Matrix3x3<Real> const& M)
{
	Real const* m = reinterpret_cast<Real const*>(&M);

    return Matrix3x3<Real>
    {
        m[1 * 3 + 1]*m[2 * 3 + 2] - m[1 * 3 + 2]*m[2 * 3 + 1],
		m[0 * 3 + 2]*m[2 * 3 + 1] - m[0 * 3 + 1]*m[2 * 3 + 2],
        m[0 * 3 + 1]*m[1 * 3 + 2] - m[0 * 3 + 2]*m[1 * 3 + 1],
        m[1 * 3 + 2]*m[2 * 3 + 0] - m[1 * 3 + 0]*m[2 * 3 + 2],
        m[0 * 3 + 0]*m[2 * 3 + 2] - m[0 * 3 + 2]*m[2 * 3 + 0],
        m[0 * 3 + 2]*m[1 * 3 + 0] - m[0 * 3 + 0]*m[1 * 3 + 2],
        m[1 * 3 + 0]*m[2 * 3 + 1] - m[1 * 3 + 1]*m[2 * 3 + 0],
        m[0 * 3 + 1]*m[2 * 3 + 0] - m[0 * 3 + 0]*m[2 * 3 + 1],
        m[0 * 3 + 0]*m[1 * 3 + 1] - m[0 * 3 + 1]*m[1 * 3 + 0]
    };
}

template <typename Real>
Real Determinant(Matrix3x3<Real> const& M)
{
	Real const* m = reinterpret_cast<Real const*>(&M);

    Real c00 = m[1 * 3 + 1]*m[2 * 3 + 2] - m[1 * 3 + 2]*m[2 * 3 + 1];
    Real c10 = m[1 * 3 + 2]*m[2 * 3 + 0] - m[1 * 3 + 0]*m[2 * 3 + 2];
    Real c20 = m[1 * 3 + 0]*m[2 * 3 + 1] - m[1 * 3 + 1]*m[2 * 3 + 0];
    Real det = m[0 * 3 + 0]*c00 + m[0 * 3 + 1]*c10 + m[0 * 3 + 2]*c20;
    return det;
}

template <typename Real>
Real Trace(Matrix3x3<Real> const& M)
{
	Real const* m = reinterpret_cast<Real const*>(&M);

    Real trace = m[0 * 3 + 0] + m[1 * 3 + 1] + m[2 * 3 + 2];
    return trace;
}


#endif