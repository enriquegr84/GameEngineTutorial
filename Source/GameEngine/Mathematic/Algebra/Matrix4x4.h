// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef MATRIX4_H
#define MATRIX4_H

#include "Mathematic/MathematicStd.h"

#include "Matrix.h"
#include "Vector4.h"

// Template alias for convenience.
template <typename Real>
using Matrix4x4 = Matrix<4, 4, Real>;

// Geometric operations.
template <typename Real>
Matrix4x4<Real> Inverse(Matrix4x4<Real> const& M,
    bool* reportInvertibility = nullptr);

template <typename Real>
Matrix4x4<Real> Adjoint(Matrix4x4<Real> const& M);

template <typename Real>
Real Determinant(Matrix4x4<Real> const& M);

template <typename Real>
Real Trace(Matrix4x4<Real> const& M);

// Special matrices.  In the comments, the matrices are shown using the
// GE_USE_MAT_VEC multiplication convention.

// The projection plane is Dot(N,X-P) = 0 where N is a 3-by-1 unit-length
// normal vector and P is a 3-by-1 point on the plane.  The projection is
// oblique to the plane, in the direction of the 3-by-1 vector D.  Necessarily
// Dot(N,D) is not zero for this projection to make sense.  Given a 3-by-1
// point U, compute the intersection of the line U+t*D with the plane to
// obtain t = -Dot(N,U-P)/Dot(N,D); then
//
//   projection(U) = P + [I - D*N^T/Dot(N,D)]*(U-P)
//
// A 4-by-4 homogeneous transformation representing the projection is
//
//       +-                               -+
//   M = | D*N^T - Dot(N,D)*I   -Dot(N,P)D |
//       |          0^T          -Dot(N,D) |
//       +-                               -+
//
// where M applies to [U^T 1]^T by M*[U^T 1]^T.  The matrix is chosen so
// that m[3][3] > 0 whenever Dot(N,D) < 0; the projection is onto the
// "positive side" of the plane.
template <typename Real>
Matrix4x4<Real> MakeObliqueProjection(Vector4<Real> const& origin,
    Vector4<Real> const& normal, Vector4<Real> const& direction);

// The perspective projection of a point onto a plane is
//
//     +-                                                 -+
// M = | Dot(N,E-P)*I - E*N^T    -(Dot(N,E-P)*I - E*N^T)*E |
//     |        -N^t                      Dot(N,E)         |
//     +-                                                 -+
//
// where E is the eye point, P is a point on the plane, and N is a
// unit-length plane normal.
template <typename Real>
Matrix4x4<Real> MakePerspectiveProjection(Vector4<Real> const& origin,
    Vector4<Real> const& normal, Vector4<Real> const& eye);

// The reflection of a point through a plane is
//     +-                         -+
// M = | I-2*N*N^T    2*Dot(N,P)*N |
//     |     0^T            1      |
//     +-                         -+
//
// where P is a point on the plane and N is a unit-length plane normal.

template <typename Real>
Matrix4x4<Real> MakeReflection(Vector4<Real> const& origin, Vector4<Real> const& normal);


template <typename Real>
Matrix4x4<Real> Inverse(Matrix4x4<Real> const& M, bool* reportInvertibility)
{
	Real const* m = reinterpret_cast<Real const*>(&M);

    Matrix4x4<Real> inverse;
    bool invertible;
    Real a0 = m[0 * 4 + 0] * m[1 * 4 + 1] - m[0 * 4 + 1] * m[1 * 4 + 0];
    Real a1 = m[0 * 4 + 0] * m[1 * 4 + 2] - m[0 * 4 + 2] * m[1 * 4 + 0];
    Real a2 = m[0 * 4 + 0] * m[1 * 4 + 3] - m[0 * 4 + 3] * m[1 * 4 + 0];
    Real a3 = m[0 * 4 + 1] * m[1 * 4 + 2] - m[0 * 4 + 2] * m[1 * 4 + 1];
    Real a4 = m[0 * 4 + 1] * m[1 * 4 + 3] - m[0 * 4 + 3] * m[1 * 4 + 1];
    Real a5 = m[0 * 4 + 2] * m[1 * 4 + 3] - m[0 * 4 + 3] * m[1 * 4 + 2];
    Real b0 = m[2 * 4 + 0] * m[3 * 4 + 1] - m[2 * 4 + 1] * m[3 * 4 + 0];
    Real b1 = m[2 * 4 + 0] * m[3 * 4 + 2] - m[2 * 4 + 2] * m[3 * 4 + 0];
    Real b2 = m[2 * 4 + 0] * m[3 * 4 + 3] - m[2 * 4 + 3] * m[3 * 4 + 0];
    Real b3 = m[2 * 4 + 1] * m[3 * 4 + 2] - m[2 * 4 + 2] * m[3 * 4 + 1];
    Real b4 = m[2 * 4 + 1] * m[3 * 4 + 3] - m[2 * 4 + 3] * m[3 * 4 + 1];
    Real b5 = m[2 * 4 + 2] * m[3 * 4 + 3] - m[2 * 4 + 3] * m[3 * 4 + 2];
    Real det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
    if (det != (Real)0)
    {
        Real invDet = ((Real)1) / det;
        inverse = Matrix4x4<Real>
        {
            (+m[1 * 4 + 1] * b5 - m[1 * 4 + 2] * b4 + m[1 * 4 + 3] * b3) * invDet,
            (-m[0 * 4 + 1] * b5 + m[0 * 4 + 2] * b4 - m[0 * 4 + 3] * b3) * invDet,
            (+m[3 * 4 + 1] * a5 - m[3 * 4 + 2] * a4 + m[3 * 4 + 3] * a3) * invDet,
            (-m[2 * 4 + 1] * a5 + m[2 * 4 + 2] * a4 - m[2 * 4 + 3] * a3) * invDet,
            (-m[1 * 4 + 0] * b5 + m[1 * 4 + 2] * b2 - m[1 * 4 + 3] * b1) * invDet,
            (+m[0 * 4 + 0] * b5 - m[0 * 4 + 2] * b2 + m[0 * 4 + 3] * b1) * invDet,
            (-m[3 * 4 + 0] * a5 + m[3 * 4 + 2] * a2 - m[3 * 4 + 3] * a1) * invDet,
            (+m[2 * 4 + 0] * a5 - m[2 * 4 + 2] * a2 + m[2 * 4 + 3] * a1) * invDet,
            (+m[1 * 4 + 0] * b4 - m[1 * 4 + 1] * b2 + m[1 * 4 + 3] * b0) * invDet,
            (-m[0 * 4 + 0] * b4 + m[0 * 4 + 1] * b2 - m[0 * 4 + 3] * b0) * invDet,
            (+m[3 * 4 + 0] * a4 - m[3 * 4 + 1] * a2 + m[3 * 4 + 3] * a0) * invDet,
            (-m[2 * 4 + 0] * a4 + m[2 * 4 + 1] * a2 - m[2 * 4 + 3] * a0) * invDet,
            (-m[1 * 4 + 0] * b3 + m[1 * 4 + 1] * b1 - m[1 * 4 + 2] * b0) * invDet,
            (+m[0 * 4 + 0] * b3 - m[0 * 4 + 1] * b1 + m[0 * 4 + 2] * b0) * invDet,
            (-m[3 * 4 + 0] * a3 + m[3 * 4 + 1] * a1 - m[3 * 4 + 2] * a0) * invDet,
            (+m[2 * 4 + 0] * a3 - m[2 * 4 + 1] * a1 + m[2 * 4 + 2] * a0) * invDet
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
Matrix4x4<Real> Adjoint(Matrix4x4<Real> const& M)
{
	Real const* m = reinterpret_cast<Real const*>(&M);

    Real a0 = m[0 * 4 + 0] * m[1 * 4 + 1] - m[0 * 4 + 1] * m[1 * 4 + 0];
    Real a1 = m[0 * 4 + 0] * m[1 * 4 + 2] - m[0 * 4 + 2] * m[1 * 4 + 0];
    Real a2 = m[0 * 4 + 0] * m[1 * 4 + 3] - m[0 * 4 + 3] * m[1 * 4 + 0];
    Real a3 = m[0 * 4 + 1] * m[1 * 4 + 2] - m[0 * 4 + 2] * m[1 * 4 + 1];
    Real a4 = m[0 * 4 + 1] * m[1 * 4 + 3] - m[0 * 4 + 3] * m[1 * 4 + 1];
    Real a5 = m[0 * 4 + 2] * m[1 * 4 + 3] - m[0 * 4 + 3] * m[1 * 4 + 2];
    Real b0 = m[2 * 4 + 0] * m[3 * 4 + 1] - m[2 * 4 + 1] * m[3 * 4 + 0];
    Real b1 = m[2 * 4 + 0] * m[3 * 4 + 2] - m[2 * 4 + 2] * m[3 * 4 + 0];
    Real b2 = m[2 * 4 + 0] * m[3 * 4 + 3] - m[2 * 4 + 3] * m[3 * 4 + 0];
    Real b3 = m[2 * 4 + 1] * m[3 * 4 + 2] - m[2 * 4 + 2] * m[3 * 4 + 1];
    Real b4 = m[2 * 4 + 1] * m[3 * 4 + 3] - m[2 * 4 + 3] * m[3 * 4 + 1];
    Real b5 = m[2 * 4 + 2] * m[3 * 4 + 3] - m[2 * 4 + 3] * m[3 * 4 + 2];

    return Matrix4x4<Real>
    {
        +m[1 * 4 + 1] * b5 - m[1 * 4 + 2] * b4 + m[1 * 4 + 3] * b3,
        -m[0 * 4 + 1] * b5 + m[0 * 4 + 2] * b4 - m[0 * 4 + 3] * b3,
        +m[3 * 4 + 1] * a5 - m[3 * 4 + 2] * a4 + m[3 * 4 + 3] * a3,
        -m[2 * 4 + 1] * a5 + m[2 * 4 + 2] * a4 - m[2 * 4 + 3] * a3,
        -m[1 * 4 + 0] * b5 + m[1 * 4 + 2] * b2 - m[1 * 4 + 3] * b1,
        +m[0 * 4 + 0] * b5 - m[0 * 4 + 2] * b2 + m[0 * 4 + 3] * b1,
        -m[3 * 4 + 0] * a5 + m[3 * 4 + 2] * a2 - m[3 * 4 + 3] * a1,
        +m[2 * 4 + 0] * a5 - m[2 * 4 + 2] * a2 + m[2 * 4 + 3] * a1,
        +m[1 * 4 + 0] * b4 - m[1 * 4 + 1] * b2 + m[1 * 4 + 3] * b0,
        -m[0 * 4 + 0] * b4 + m[0 * 4 + 1] * b2 - m[0 * 4 + 3] * b0,
        +m[3 * 4 + 0] * a4 - m[3 * 4 + 1] * a2 + m[3 * 4 + 3] * a0,
        -m[2 * 4 + 0] * a4 + m[2 * 4 + 1] * a2 - m[2 * 4 + 3] * a0,
        -m[1 * 4 + 0] * b3 + m[1 * 4 + 1] * b1 - m[1 * 4 + 2] * b0,
        +m[0 * 4 + 0] * b3 - m[0 * 4 + 1] * b1 + m[0 * 4 + 2] * b0,
        -m[3 * 4 + 0] * a3 + m[3 * 4 + 1] * a1 - m[3 * 4 + 2] * a0,
        +m[2 * 4 + 0] * a3 - m[2 * 4 + 1] * a1 + m[2 * 4 + 2] * a0
    };
}

template <typename Real>
Real Determinant(Matrix4x4<Real> const& M)
{
	Real const* m = reinterpret_cast<Real const*>(&M);

    Real a0 = m[0 * 4 + 0] * m[1 * 4 + 1] - m[0 * 4 + 1] * m[1 * 4 + 0];
    Real a1 = m[0 * 4 + 0] * m[1 * 4 + 2] - m[0 * 4 + 2] * m[1 * 4 + 0];
    Real a2 = m[0 * 4 + 0] * m[1 * 4 + 3] - m[0 * 4 + 3] * m[1 * 4 + 0];
    Real a3 = m[0 * 4 + 1] * m[1 * 4 + 2] - m[0 * 4 + 2] * m[1 * 4 + 1];
    Real a4 = m[0 * 4 + 1] * m[1 * 4 + 3] - m[0 * 4 + 3] * m[1 * 4 + 1];
    Real a5 = m[0 * 4 + 2] * m[1 * 4 + 3] - m[0 * 4 + 3] * m[1 * 4 + 2];
    Real b0 = m[2 * 4 + 0] * m[3 * 4 + 1] - m[2 * 4 + 1] * m[3 * 4 + 0];
    Real b1 = m[2 * 4 + 0] * m[3 * 4 + 2] - m[2 * 4 + 2] * m[3 * 4 + 0];
    Real b2 = m[2 * 4 + 0] * m[3 * 4 + 3] - m[2 * 4 + 3] * m[3 * 4 + 0];
    Real b3 = m[2 * 4 + 1] * m[3 * 4 + 2] - m[2 * 4 + 2] * m[3 * 4 + 1];
    Real b4 = m[2 * 4 + 1] * m[3 * 4 + 3] - m[2 * 4 + 3] * m[3 * 4 + 1];
    Real b5 = m[2 * 4 + 2] * m[3 * 4 + 3] - m[2 * 4 + 3] * m[3 * 4 + 2];
    Real det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
    return det;
}

template <typename Real>
Real Trace(Matrix4x4<Real> const& M)
{
	Real const* m = reinterpret_cast<Real const*>(&M);

    Real trace = m[0 * 4 + 0] + m[1 * 4 + 1] + m[2 * 4 + 2] + m[3 * 4 + 3];
    return trace;
}

template <typename Real>
Matrix4x4<Real> MakeObliqueProjection(Vector4<Real> const& origin,
    Vector4<Real> const& normal, Vector4<Real> const& direction)
{
    Matrix4x4<Real> M;
	Real* m = reinterpret_cast<Real*>(&M);

    Real const zero = (Real)0;
    Real dotND = Dot(normal, direction);
    Real dotNO = Dot(origin, normal);

#if defined(GE_USE_MAT_VEC)
    m[0 * 4 + 0] = direction[0] * normal[0] - dotND;
    m[0 * 4 + 1] = direction[0] * normal[1];
    m[0 * 4 + 2] = direction[0] * normal[2];
    m[0 * 4 + 3] = -dotNO * direction[0];
    m[1 * 4 + 0] = direction[1] * normal[0];
    m[1 * 4 + 1] = direction[1] * normal[1] - dotND;
    m[1 * 4 + 2] = direction[1] * normal[2];
    m[1 * 4 + 3] = -dotNO * direction[1];
    m[2 * 4 + 0] = direction[2] * normal[0];
    m[2 * 4 + 1] = direction[2] * normal[1];
    m[2 * 4 + 2] = direction[2] * normal[2] - dotND;
    m[2 * 4 + 3] = -dotNO * direction[2];
    m[3 * 4 + 0] = zero;
    m[3 * 4 + 1] = zero;
    m[3 * 4 + 2] = zero;
    m[3 * 4 + 3] = -dotND;
#else
    m[0 * 4 + 0] = direction[0] * normal[0] - dotND;
    m[1 * 4 + 0] = direction[0] * normal[1];
    m[2 * 4 + 0] = direction[0] * normal[2];
    m[3 * 4 + 0] = -dotNO * direction[0];
    m[0 * 4 + 1] = direction[1] * normal[0];
    m[1 * 4 + 1] = direction[1] * normal[1] - dotND;
    m[2 * 4 + 1] = direction[1] * normal[2];
    m[3 * 4 + 1] = -dotNO * direction[1];
    m[0 * 4 + 2] = direction[2] * normal[0];
    m[1 * 4 + 2] = direction[2] * normal[1];
    m[2 * 4 + 2] = direction[2] * normal[2] - dotND;
    m[3 * 4 + 2] = -dotNO * direction[2];
    m[0 * 4 + 2] = zero;
    m[1 * 4 + 3] = zero;
    m[2 * 4 + 3] = zero;
    m[3 * 4 + 3] = -dotND;
#endif

    return M;
}

template <typename Real>
Matrix4x4<Real> MakePerspectiveProjection(Vector4<Real> const& origin,
    Vector4<Real> const& normal, Vector4<Real> const& eye)
{
    Matrix4x4<Real> M;
	Real* m = reinterpret_cast<Real*>(&M);

    Real dotND = Dot(normal, eye - origin);

#if defined(GE_USE_MAT_VEC)
    m[0 * 4 + 0] = dotND - eye[0] * normal[0];
    m[0 * 4 + 1] = -eye[0] * normal[1];
    m[0 * 4 + 2] = -eye[0] * normal[2];
    m[0 * 4 + 3] = -(m[0 * 4 + 0] * eye[0] + m[0 * 4 + 1] * eye[1] + m[0 * 4 + 2] * eye[2]);
    m[1 * 4 + 0] = -eye[1] * normal[0];
    m[1 * 4 + 1] = dotND - eye[1] * normal[1];
    m[1 * 4 + 2] = -eye[1] * normal[2];
    m[1 * 4 + 3] = -(m[1 * 4 + 0] * eye[0] + m[1 * 4 + 1] * eye[1] + m[1 * 4 + 2] * eye[2]);
    m[2 * 4 + 0] = -eye[2] * normal[0];
    m[2 * 4 + 1] = -eye[2] * normal[1];
    m[2 * 4 + 2] = dotND - eye[2] * normal[2];
    m[2 * 4 + 3] = -(m[2 * 4 + 0] * eye[0] + m[2 * 4 + 1] * eye[1] + m[2 * 4 + 2] * eye[2]);
    m[3 * 4 + 0] = -normal[0];
    m[3 * 4 + 1] = -normal[1];
    m[3 * 4 + 2] = -normal[2];
    m[3 * 4 + 3] = Dot(eye, normal);
#else
    m[0 * 4 + 0] = dotND - eye[0] * normal[0];
    m[1 * 4 + 0] = -eye[0] * normal[1];
    m[2 * 4 + 0] = -eye[0] * normal[2];
    m[3 * 4 + 0] = -(m[0 * 4 + 0] * eye[0] + m[0 * 4 + 1] * eye[1] + m[0 * 4 + 2] * eye[2]);
    m[0 * 4 + 1] = -eye[1] * normal[0];
    m[1 * 4 + 1] = dotND - eye[1] * normal[1];
    m[2 * 4 + 1] = -eye[1] * normal[2];
    m[3 * 4 + 1] = -(m[1 * 4 + 0] * eye[0] + m[1 * 4 + 1] * eye[1] + m[1 * 4 + 2] * eye[2]);
    m[0 * 4 + 2] = -eye[2] * normal[0];
    m[1 * 4 + 2] = -eye[2] * normal[1];
    m[2 * 4 + 2] = dotND - eye[2] * normal[2];
    m[3 * 4 + 2] = -(m[2 * 4 + 0] * eye[0] + m[2 * 4 + 1] * eye[1] + m[2 * 4 + 2] * eye[2]);
    m[0 * 4 + 3] = -normal[0];
    m[1 * 4 + 3] = -normal[1];
    m[2 * 4 + 3] = -normal[2];
    m[3 * 4 + 3] = Dot(eye, normal);
#endif

    return M;
}

template <typename Real>
Matrix4x4<Real> MakeReflection(Vector4<Real> const& origin, Vector4<Real> const& normal)
{
    Matrix4x4<Real> M;
	Real* m = reinterpret_cast<Real*>(&M);

    Real const zero = (Real)0, one = (Real)1, two = (Real)2;
    Real twoDotNO = two * Dot(origin, normal);

#if defined(GE_USE_MAT_VEC)
    m[0 * 4 + 0] = one - two * normal[0] * normal[0];
    m[0 * 4 + 1] = -two * normal[0] * normal[1];
    m[0 * 4 + 2] = -two * normal[0] * normal[2];
    m[0 * 4 + 3] = twoDotNO * normal[0];
    m[1 * 4 + 0] = m[0 * 4 + 1];
    m[1 * 4 + 1] = one - two * normal[1] * normal[1];
    m[1 * 4 + 2] = -two * normal[1] * normal[2];
    m[1 * 4 + 3] = twoDotNO * normal[1];
    m[2 * 4 + 0] = m[0 * 4 + 2];
    m[2 * 4 + 1] = m[1 * 4 + 2];
    m[2 * 4 + 2] = one - two * normal[2] * normal[2];
    m[2 * 4 + 3] = twoDotNO * normal[2];
    m[3 * 4 + 0] = zero;
    m[3 * 4 + 1] = zero;
    m[3 * 4 + 2] = zero;
    m[3 * 4 + 3] = one;
#else
    m[0 * 4 + 0] = one - two * normal[0] * normal[0];
    m[1 * 4 + 0] = -two * normal[0] * normal[1];
    m[2 * 4 + 0] = -two * normal[0] * normal[2];
    m[3 * 4 + 0] = twoDotNO * normal[0];
    m[0 * 4 + 1] = m[1 * 4 + 0];
    m[1 * 4 + 1] = one - two * normal[1] * normal[1];
    m[2 * 4 + 1] = -two * normal[1] * normal[2];
    m[3 * 4 + 1] = twoDotNO * normal[1];
    m[0 * 4 + 2] = m[2 * 4 + 0];
    m[1 * 4 + 2] = m[2 * 4 + 1];
    m[2 * 4 + 2] = one - two * normal[2] * normal[2];
    m[3 * 4 + 2] = twoDotNO * normal[2];
    m[0 * 4 + 3] = zero;
    m[1 * 4 + 3] = zero;
    m[2 * 4 + 3] = zero;
    m[3 * 4 + 3] = one;
#endif

    return M;
}


#endif