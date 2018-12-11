// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef ROTATION_H
#define ROTATION_H

#include "Mathematic/MathematicStd.h"

#include "AxisAngle.h"
#include "EulerAngles.h"
#include "Matrix.h"
#include "Quaternion.h"

#include "Mathematic/Function/Constants.h"

// Conversions among various representations of rotations.  The value of
// N must be 3 or 4.  The latter case supports affine algebra when you use
// 4-tuple vectors (w-component is 1 for points and 0 for vector) and 4x4
// matrices for affine transformations.  Rotation axes must be unit length.
// The angles are in radians.  The Euler angles are in world coordinates;
// we have not yet added support for body coordinates.

template <int N, typename Real>
class Rotation
{
public:
    // Create rotations from various representations.
    Rotation(Matrix<N,N,Real> const& matrix);
    Rotation(Quaternion<Real> const& quaternion);
    Rotation(AxisAngle<N,Real> const& axisAngle);
    Rotation(EulerAngles<Real> const& eulerAngles);

    // Convert one representation to another.
	operator Matrix<N, N, Real>() const;
	operator Quaternion<Real>() const;
	operator AxisAngle<N, Real>() const;
    EulerAngles<Real> const& operator()(int i0, int i1, int i2) const;

private:
    enum RepresentationType
    {
        IS_MATRIX,
        IS_QUATERNION,
        IS_AXIS_ANGLE,
        IS_EULER_ANGLES
    };

    RepresentationType mType;
    mutable Matrix<N,N,Real> mMatrix;
    mutable Quaternion<Real> mQuaternion;
    mutable AxisAngle<N,Real> mAxisAngle;
    mutable EulerAngles<Real> mEulerAngles;

    // Convert a rotation matrix to a quaternion.
    //
    // x^2 = (+r00 - r11 - r22 + 1)/4
    // y^2 = (-r00 + r11 - r22 + 1)/4
    // z^2 = (-r00 - r11 + r22 + 1)/4
    // w^2 = (+r00 + r11 + r22 + 1)/4
    // x^2 + y^2 = (1 - r22)/2
    // z^2 + w^2 = (1 + r22)/2
    // y^2 - x^2 = (r11 - r00)/2
    // w^2 - z^2 = (r11 + r00)/2
    // x*y = (r01 + r10)/4
    // x*z = (r02 + r20)/4
    // y*z = (r12 + r21)/4
    // [GE_USE_MAT_VEC]
    //   x*w = (r21 - r12)/4
    //   y*w = (r02 - r20)/4
    //   z*w = (r10 - r01)/4
    // [GE_USE_VEC_MAT]
    //   x*w = (r12 - r21)/4
    //   y*w = (r20 - r02)/4
    //   z*w = (r01 - r10)/4
    //
    // If Q is the 4x1 column vector (x,y,z,w), the previous equations give us
    //         +-                  -+
    //         | x*x  x*y  x*z  x*w |
    // Q*Q^T = | y*x  y*y  y*z  y*w |
    //         | z*x  z*y  z*z  z*w |
    //         | w*x  w*y  w*z  w*w |
    //         +-                  -+
    // The code extracts the row of maximum length, normalizing it to obtain
    // the result q.
    static void Convert(Matrix<N,N,Real> const& r, Quaternion<Real>& q);

    // Convert a quaterion q = x*i + y*j + z*k + w to a rotation matrix.
    // [GE_USE_MAT_VEC]
    //     +-           -+   +-                                     -+
    // R = | r00 r01 r02 | = | 1-2y^2-2z^2  2(xy-zw)     2(xz+yw)    |
    //     | r10 r11 r12 |   | 2(xy+zw)     1-2x^2-2z^2  2(yz-xw)    |
    //     | r20 r21 r22 |   | 2(xz-yw)     2(yz+xw)     1-2x^2-2y^2 |
    //     +-           -+   +-                                     -+
    // [GE_USE_VEC_MAT]
    //     +-           -+   +-                                     -+
    // R = | r00 r01 r02 | = | 1-2y^2-2z^2  2(xy+zw)     2(xz-yw)    |
    //     | r10 r11 r12 |   | 2(xy-zw)     1-2x^2-2z^2  2(yz+xw)    |
    //     | r20 r21 r22 |   | 2(xz+yw)     2(yz-xw)     1-2x^2-2y^2 |
    //     +-           -+   +-                                     -+
    static void Convert(Quaternion<Real> const& q, Matrix<N,N,Real>& r);

    // Convert a rotation matrix to an axis-angle pair.  Let (x0,x1,x2) be the
    // axis let t be an angle of rotation.  The rotation matrix is
    // [GE_USE_MAT_VEC]
    //   R = I + sin(t)*S + (1-cos(t))*S^2
    // or
    // [GE_USE_VEC_MAT]
    //   R = I - sin(t)*S + (1-cos(t))*S^2
    // where I is the identity and S = {{0,-x2,x1},{x2,0,-x0},{-x1,x0,0}}
    // where the inner-brace triples are the rows of the matrix.  If t > 0,
    // R represents a counterclockwise rotation; see the comments for the
    // constructor Matrix3x3(axis,angle).  It may be shown that cos(t) =
    // (trace(R)-1)/2 and R - Transpose(R) = 2*sin(t)*S.  As long as sin(t) is
    // not zero, we may solve for S in the second equation, which produces the
    // axis direction U = (S21,S02,S10).  When t = 0, the rotation is the
    // identity, in which case any axis direction is valid; we choose (1,0,0).
    // When t = pi, it must be that R - Transpose(R) = 0, which prevents us
    // from extracting the axis.  Instead, note that (R+I)/2 = I+S^2 = U*U^T,
    // where U is a unit-length axis direction.
    static void Convert(Matrix<N,N,Real> const& r, AxisAngle<N,Real>& a);

    // Convert an axis-angle pair to a rotation matrix.  Assuming (x0,x1,x2)
    // is for a right-handed world (x0 to right, x1 up, x2 out of plane of
    // page), a positive angle corresponds to a counterclockwise rotation from
    // the perspective of an observer looking at the origin of the plane of
    // rotation and having view direction the negative of the axis direction.
    // The coordinate-axis rotations are the following, where
    // unit(0) = (1,0,0), unit(1) = (0,1,0), unit(2) = (0,0,1),
    // [GE_USE_MAT_VEC]
    //   R(unit(0),t) = {{ 1, 0, 0}, { 0, c,-s}, { 0, s, c}}
    //   R(unit(1),t) = {{ c, 0, s}, { 0, 1, 0}, {-s, 0, c}}
    //   R(unit(2),t) = {{ c,-s, 0}, { s, c, 0}, { 0, 0, 1}}
    // or
    // [GE_USE_VEC_MAT]
    //   R(unit(0),t) = {{ 1, 0, 0}, { 0, c, s}, { 0,-s, c}}
    //   R(unit(1),t) = {{ c, 0,-s}, { 0, 1, 0}, { s, 0, c}}
    //   R(unit(2),t) = {{ c, s, 0}, {-s, c, 0}, { 0, 0, 1}}
    // where c = cos(t), s = sin(t), and the inner-brace triples are rows of
    // the matrix.  The general matrix is
    // [GE_USE_MAT_VEC]
    //       +-                                                          -+
    //   R = | (1-c)*x0^2  + c     (1-c)*x0*x1 - s*x2  (1-c)*x0*x2 + s*x1 |
    //       | (1-c)*x0*x1 + s*x2  (1-c)*x1^2  + c     (1-c)*x1*x2 - s*x0 |
    //       | (1-c)*x0*x2 - s*x1  (1-c)*x1*x2 + s*x0  (1-c)*x2^2  + c    |
    //       +-                                                          -+
    // [GE_USE_VEC_MAT]
    //       +-                                                          -+
    //   R = | (1-c)*x0^2  + c     (1-c)*x0*x1 + s*x2  (1-c)*x0*x2 - s*x1 |
    //       | (1-c)*x0*x1 - s*x2  (1-c)*x1^2  + c     (1-c)*x1*x2 + s*x0 |
    //       | (1-c)*x0*x2 + s*x1  (1-c)*x1*x2 - s*x0  (1-c)*x2^2  + c    |
    //       +-                                                          -+
    static void Convert(AxisAngle<N,Real> const& a, Matrix<N,N,Real>& r);

    // Convert a rotation matrix to Euler angles.  Factorization into Euler
    // angles is not necessarily unique.  If the result is ER_NOT_UNIQUE_SUM,
    // then the multiple solutions occur because angleN2+angleN0 is constant.
    // If the result is ER_NOT_UNIQUE_DIF, then the multiple solutions occur
    // because angleN2-angleN0 is constant.  In either type of nonuniqueness,
    // the function returns angleN0=0.
    static void Convert(Matrix<N,N,Real> const& r, EulerAngles<Real>& e);

    // Convert Euler angles to a rotation matrix.  The three integer inputs
    // are in {0,1,2} and correspond to world directions unit(0) = (1,0,0),
    // unit(1) = (0,1,0), or unit(2) = (0,0,1).  The triples (N0,N1,N2) must
    // be in the following set,
    //   {(0,1,2),(0,2,1),(1,0,2),(1,2,0),(2,0,1),(2,1,0),
    //    (0,1,0),(0,2,0),(1,0,1),(1,2,1),(2,0,2),(2,1,2)}
    // The rotation matrix is
    //   [GE_USE_MAT_VEC]
    //     R(unit(N2),angleN2)*R(unit(N1),angleN1)*R(unit(N0),angleN0)
    // or
    //   [GE_USE_VEC_MAT]
    //     R(unit(N0),angleN0)*R(unit(N1),angleN1)*R(unit(N2),angleN2)
    // The conventions of constructor Matrix3(axis,angle) apply here as well.
    //
    // NOTE:  The reversal of order is chosen so that a rotation matrix built
    // with one multiplication convention is the transpose of the rotation
    // matrix built with the other multiplication convention.  Thus,
    //   [GE_USE_MAT_VEC]
    //     Matrix3x3<Real> R_mvconvention(N0,N1,N2,angleN0,angleN1,angleN2);
    //     Vector3<Real> V(...);
    //     Vector3<Real> U = R_mvconvention*V;  // (u0,u1,u2) = R2*R1*R0*V
    //   [GE_USE_VEC_MAT]
    //     Matrix3x3<Real> R_vmconvention(N0,N1,N2,angleN0,angleN1,angleN2);
    //     Vector3<Real> V(...);
    //     Vector3<Real> U = R_mvconvention*V;  // (u0,u1,u2) = V*R0*R1*R2
    // In either convention, you get the same 3-tuple U.
    static void Convert(EulerAngles<Real> const& e, Matrix<N,N,Real>& r);

    // Convert a quaternion to an axis-angle pair, where
    //   q = sin(angle/2)*(axis[0]*i + axis[1]*j + axis[2]*k) + cos(angle/2)
    static void Convert(Quaternion<Real> const& q, AxisAngle<N,Real>& a);

    // Convert an axis-angle pair to a quaternion, where
    //   q = sin(angle/2)*(axis[0]*i + axis[1]*j + axis[2]*k) + cos(angle/2)
    static void Convert(AxisAngle<N,Real> const& a, Quaternion<Real>& q);

    // Convert a quaternion to Euler angles.  The quaternion is converted to
    // a matrix which is then converted to Euler angles.
    static void Convert(Quaternion<Real> const& q, EulerAngles<Real>& e);

    // Convert Euler angles to a quaternion.  The Euler angles are converted
    // to a matrix which is then converted to a quaternion.
    static void Convert(EulerAngles<Real> const& e, Quaternion<Real>& q);

    // Convert an axis-angle pair to Euler angles.  The axis-angle pair is
    // converted to a quaternion which is then converted to Euler angles.
    static void Convert(AxisAngle<N,Real> const& a, EulerAngles<Real>& e);

    // Convert Euler angles to an axis-angle pair.  The Euler angles are
    // converted to a quaternion which is then converted to an axis-angle
    // pair.
    static void Convert(EulerAngles<Real> const& e, AxisAngle<N,Real>& a);
};


template <int N, typename Real>
Rotation<N, Real>::Rotation(Matrix<N, N, Real> const& matrix)
    :
    mType(IS_MATRIX),
    mMatrix(matrix)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
}

template <int N, typename Real>
Rotation<N, Real>::Rotation(Quaternion<Real> const& quaternion)
    :
    mType(IS_QUATERNION),
    mQuaternion(quaternion)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
}

template <int N, typename Real>
Rotation<N, Real>::Rotation(AxisAngle<N, Real> const& axisAngle)
    :
    mType(IS_AXIS_ANGLE),
    mAxisAngle(axisAngle)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
}

template <int N, typename Real>
Rotation<N, Real>::Rotation(EulerAngles<Real> const& eulerAngles)
    :
    mType(IS_EULER_ANGLES),
    mEulerAngles(eulerAngles)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
}

template <int N, typename Real>
Rotation<N, Real>::operator Matrix<N, N, Real>() const
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");

    switch (mType)
    {
    case IS_MATRIX:
        break;
    case IS_QUATERNION:
        Convert(mQuaternion, mMatrix);
        break;
    case IS_AXIS_ANGLE:
        Convert(mAxisAngle, mMatrix);
        break;
    case IS_EULER_ANGLES:
        Convert(mEulerAngles, mMatrix);
        break;
    }

    return mMatrix;
}

template <int N, typename Real>
Rotation<N, Real>::operator Quaternion<Real>() const
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");

    switch (mType)
    {
    case IS_MATRIX:
        Convert(mMatrix, mQuaternion);
        break;
    case IS_QUATERNION:
        break;
    case IS_AXIS_ANGLE:
        Convert(mAxisAngle, mQuaternion);
        break;
    case IS_EULER_ANGLES:
        Convert(mEulerAngles, mQuaternion);
        break;
    }

    return mQuaternion;
}

template <int N, typename Real>
Rotation<N, Real>::operator AxisAngle<N, Real>() const
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");

    switch (mType)
    {
    case IS_MATRIX:
        Convert(mMatrix, mAxisAngle);
        break;
    case IS_QUATERNION:
        Convert(mQuaternion, mAxisAngle);
        break;
    case IS_AXIS_ANGLE:
        break;
    case IS_EULER_ANGLES:
        Convert(mEulerAngles, mAxisAngle);
        break;
    }

    return mAxisAngle;
}

template <int N, typename Real>
EulerAngles<Real> const& Rotation<N, Real>::operator()(int i0, int i1, int i2) const
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");

    mEulerAngles.mAxis[0] = i0;
    mEulerAngles.mAxis[1] = i1;
    mEulerAngles.mAxis[2] = i2;

    switch (mType)
    {
    case IS_MATRIX:
        Convert(mMatrix, mEulerAngles);
        break;
    case IS_QUATERNION:
        Convert(mQuaternion, mEulerAngles);
        break;
    case IS_AXIS_ANGLE:
        Convert(mAxisAngle, mEulerAngles);
        break;
    case IS_EULER_ANGLES:
        break;
    }

    return mEulerAngles;
}

template <int N, typename Real>
void Rotation<N, Real>::Convert(Matrix<N, N, Real> const& r, Quaternion<Real>& q)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
	Real const* rot = reinterpret_cast<Real const*>(&r);

    Real r22 = rot[2 * 4 + 2];
    if (r22 <= (Real)0)  // x^2 + y^2 >= z^2 + w^2
    {
        Real dif10 = rot[1 * 4 + 1] - rot[0 * 4 + 0];
        Real omr22 = (Real)1 - r22;
        if (dif10 <= (Real)0)  // x^2 >= y^2
        {
            Real fourXSqr = omr22 - dif10;
            Real inv4x = ((Real)0.5) / sqrt(fourXSqr);
            q[0] = fourXSqr*inv4x;
            q[1] = (rot[0 * 4 + 1] + rot[1 * 4 + 0])*inv4x;
            q[2] = (rot[0 * 4 + 2] + rot[2 * 4 + 0])*inv4x;
#if defined(GE_USE_MAT_VEC)
            q[3] = (rot[2 * 4 + 1] - rot[1 * 4 + 2])*inv4x;
#else
            q[3] = (rot[1 * 4 + 2] - rot[2 * 4 + 1])*inv4x;
#endif
        }
        else  // y^2 >= x^2
        {
            Real fourYSqr = omr22 + dif10;
            Real inv4y = ((Real)0.5) / sqrt(fourYSqr);
            q[0] = (rot[0 * 4 + 1] + rot[1 * 4 + 0])*inv4y;
            q[1] = fourYSqr*inv4y;
            q[2] = (rot[1 * 4 + 2] + rot[2 * 4 + 1])*inv4y;
#if defined(GE_USE_MAT_VEC)
            q[3] = (rot[0 * 4 + 2] - rot[2 * 4 + 0])*inv4y;
#else
            q[3] = (rot[2 * 4 + 0] - rot[0 * 4 + 2])*inv4y;
#endif
        }
    }
    else  // z^2 + w^2 >= x^2 + y^2
    {
        Real sum10 = rot[1 * 4 + 1] + rot[0 * 4 + 0];
        Real opr22 = (Real)1 + r22;
        if (sum10 <= (Real)0)  // z^2 >= w^2
        {
            Real fourZSqr = opr22 - sum10;
            Real inv4z = ((Real)0.5) / sqrt(fourZSqr);
            q[0] = (rot[0 * 4 + 2] + rot[2 * 4 + 0])*inv4z;
            q[1] = (rot[1 * 4 + 2] + rot[2 * 4 + 1])*inv4z;
            q[2] = fourZSqr*inv4z;
#if defined(GE_USE_MAT_VEC)
            q[3] = (rot[1 * 4 + 0] - rot[0 * 4 + 1])*inv4z;
#else
            q[3] = (rot[0 * 4 + 1] - rot[1 * 4 + 0])*inv4z;
#endif
        }
        else  // w^2 >= z^2
        {
            Real fourWSqr = opr22 + sum10;
            Real inv4w = ((Real)0.5) / sqrt(fourWSqr);
#if defined(GE_USE_MAT_VEC)
            q[0] = (rot[2 * 4 + 1] - rot[1 * 4 + 2])*inv4w;
            q[1] = (rot[0 * 4 + 2] - rot[2 * 4 + 0])*inv4w;
            q[2] = (rot[1 * 4 + 0] - rot[0 * 4 + 1])*inv4w;
#else
            q[0] = (rot[1 * 4 + 2] - rot[2 * 4 + 1])*inv4w;
            q[1] = (rot[2 * 4 + 0] - rot[0 * 4 + 2])*inv4w;
            q[2] = (rot[0 * 4 + 1] - rot[1 * 4 + 0])*inv4w;
#endif
            q[3] = fourWSqr*inv4w;
        }
    }
}

template <int N, typename Real>
void Rotation<N, Real>::Convert(Quaternion<Real> const& q, Matrix<N, N, Real>& r)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
	Real* rot = reinterpret_cast<Real*>(&r);

    r.MakeIdentity();

    Real twoX = ((Real)2)*q[0];
    Real twoY = ((Real)2)*q[1];
    Real twoZ = ((Real)2)*q[2];
    Real twoXX = twoX*q[0];
    Real twoXY = twoX*q[1];
    Real twoXZ = twoX*q[2];
    Real twoXW = twoX*q[3];
    Real twoYY = twoY*q[1];
    Real twoYZ = twoY*q[2];
    Real twoYW = twoY*q[3];
    Real twoZZ = twoZ*q[2];
    Real twoZW = twoZ*q[3];

#if defined(GE_USE_MAT_VEC)
    rot[0 * 4 + 0] = (Real)1 - twoYY - twoZZ;
    rot[0 * 4 + 1] = twoXY - twoZW;
    rot[0 * 4 + 2] = twoXZ + twoYW;
    rot[1 * 4 + 0] = twoXY + twoZW;
    rot[1 * 4 + 1] = (Real)1 - twoXX - twoZZ;
    rot[1 * 4 + 2] = twoYZ - twoXW;
    rot[2 * 4 + 0] = twoXZ - twoYW;
    rot[2 * 4 + 1] = twoYZ + twoXW;
    rot[2 * 4 + 2] = (Real)1 - twoXX - twoYY;
#else
    rot[0 * 4 + 0] = (Real)1 - twoYY - twoZZ;
    rot[1 * 4 + 0] = twoXY - twoZW;
    rot[2 * 4 + 0] = twoXZ + twoYW;
    rot[0 * 4 + 1] = twoXY + twoZW;
    rot[1 * 4 + 1] = (Real)1 - twoXX - twoZZ;
    rot[2 * 4 + 1] = twoYZ - twoXW;
    rot[0 * 4 + 2] = twoXZ - twoYW;
    rot[1 * 4 + 2] = twoYZ + twoXW;
    rot[2 * 4 + 2] = (Real)1 - twoXX - twoYY;
#endif
}

template <int N, typename Real>
void Rotation<N, Real>::Convert(Matrix<N, N, Real> const& r, AxisAngle<N, Real>& a)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
	Real const* rot = reinterpret_cast<Real const*>(&r);

    Real trace = rot[0 * 4 + 0] + rot[1 * 4 + 1] + rot[2 * 4 + 2];
    Real half = (Real)0.5;
    Real cs = half*(trace - (Real)1);
    cs = eastl::max(eastl::min(cs, (Real)1), (Real)-1);
    a.mAngle = acos(cs);  // The angle is in [0,pi].
    a.mAxis.MakeZero();

    if (a.mAngle > (Real)0)
    {
        if (a.mAngle < (Real)GE_C_PI)
        {
            // The angle is in (0,pi).
#if defined(GE_USE_MAT_VEC)
            a.mAxis[0] = rot[2 * 4 + 1] - rot[1 * 4 + 2];
            a.mAxis[1] = rot[0 * 4 + 2] - rot[2 * 4 + 0];
            a.mAxis[2] = rot[1 * 4 + 0] - rot[0 * 4 + 1];
            Normalize(a.mAxis);
#else
            a.mAxis[0] = rot[1 * 4 + 2] - rot[2 * 4 + 1];
            a.mAxis[1] = rot[2 * 4 + 0] - rot[0 * 4 + 2];
            a.mAxis[2] = rot[0 * 4 + 1] - rot[1 * 4 + 0];
            Normalize(a.mAxis);
#endif
        }
        else
        {
            // The angle is pi, in which case R is symmetric and
            // R+I = 2*(I+S^2) = 2*U*U^T, where U = (u0,u1,u2) is the
            // unit-length direction of the rotation axis.  Determine the
            // largest diagonal entry of R+I and normalize the
            // corresponding row to produce U.  It does not matter the
            // sign on u[d] for chosen diagonal d, because R(U,pi) = R(-U,pi).
            Real one = (Real)1;
            if (rot[0 * 4 + 0] >= rot[1 * 4 + 1])
            {
                if (rot[0 * 4 + 0] >= rot[2 * 4 + 2])
                {
                    // r00 is maximum diagonal term
                    a.mAxis[0] = rot[0 * 4 + 0] + one;
                    a.mAxis[1] = half*(rot[0 * 4 + 1] + rot[1 * 4 + 0]);
                    a.mAxis[2] = half*(rot[0 * 4 + 2] + rot[2 * 4 + 0]);
                }
                else
                {
                    // r22 is maximum diagonal term
                    a.mAxis[0] = half*(rot[2 * 4 + 0] + rot[0 * 4 + 2]);
                    a.mAxis[1] = half*(rot[2 * 4 + 1] + rot[1 * 4 + 2]);
                    a.mAxis[2] = rot[2 * 4 + 2] + one;
                }
            }
            else
            {
                if (rot[1 * 4 + 1] >= rot[2 * 4 + 2])
                {
                    // r11 is maximum diagonal term
                    a.mAxis[0] = half*(rot[1 * 4 + 0] + rot[0 * 4 + 1]);
                    a.mAxis[1] = rot[1 * 4 + 1] + one;
                    a.mAxis[2] = half*(rot[1 * 4 + 2] + rot[2 * 4 + 1]);
                }
                else
                {
                    // r22 is maximum diagonal term
                    a.mAxis[0] = half*(rot[2 * 4 + 0] + rot[0 * 4 + 2]);
                    a.mAxis[1] = half*(rot[2 * 4 + 1] + rot[1 * 4 + 2]);
                    a.mAxis[2] = rot[2 * 4 + 2] + one;
                }
            }
            Normalize(a.mAxis);
        }
    }
    else
    {
        // The angle is 0 and the matrix is the identity.  Any axis will
        // work, so choose the Unit(0) axis.
        a.mAxis[0] = (Real)1;
    }
}

template <int N, typename Real>
void Rotation<N, Real>::Convert(AxisAngle<N, Real> const& a, Matrix<N, N, Real>& r)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
	Real* rot = reinterpret_cast<Real*>(&r);

    r.MakeIdentity();

    Real cs = cos(a.mAngle);
    Real sn = sin(a.mAngle);
    Real oneMinusCos = ((Real)1) - cs;
    Real x0sqr = a.mAxis[0] * a.mAxis[0];
    Real x1sqr = a.mAxis[1] * a.mAxis[1];
    Real x2sqr = a.mAxis[2] * a.mAxis[2];
    Real x0x1m = a.mAxis[0] * a.mAxis[1] * oneMinusCos;
    Real x0x2m = a.mAxis[0] * a.mAxis[2] * oneMinusCos;
    Real x1x2m = a.mAxis[1] * a.mAxis[2] * oneMinusCos;
    Real x0Sin = a.mAxis[0] * sn;
    Real x1Sin = a.mAxis[1] * sn;
    Real x2Sin = a.mAxis[2] * sn;

#if defined(GE_USE_MAT_VEC)
    rot[0 * 4 + 0] = x0sqr*oneMinusCos + cs;
    rot[0 * 4 + 1] = x0x1m - x2Sin;
    rot[0 * 4 + 2] = x0x2m + x1Sin;
    rot[1 * 4 + 0] = x0x1m + x2Sin;
    rot[1 * 4 + 1] = x1sqr*oneMinusCos + cs;
    rot[1 * 4 + 2] = x1x2m - x0Sin;
    rot[2 * 4 + 0] = x0x2m - x1Sin;
    rot[2 * 4 + 1] = x1x2m + x0Sin;
    rot[2 * 4 + 2] = x2sqr*oneMinusCos + cs;
#else
    rot[0 * 4 + 0] = x0sqr*oneMinusCos + cs;
    rot[1 * 4 + 0] = x0x1m - x2Sin;
    rot[2 * 4 + 0] = x0x2m + x1Sin;
    rot[0 * 4 + 1] = x0x1m + x2Sin;
    rot[1 * 4 + 1] = x1sqr*oneMinusCos + cs;
    rot[2 * 4 + 1] = x1x2m - x0Sin;
    rot[0 * 4 + 2] = x0x2m - x1Sin;
    rot[1 * 4 + 2] = x1x2m + x0Sin;
    rot[2 * 4 + 2] = x2sqr*oneMinusCos + cs;
#endif
}

template <int N, typename Real>
void Rotation<N, Real>::Convert(Matrix<N, N, Real> const& r, EulerAngles<Real>& e)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");
	Real const* rot = reinterpret_cast<Real const*>(&r);

    if (0 <= e.mAxis[0] && e.mAxis[0] < 3
        && 0 <= e.mAxis[1] && e.mAxis[1] < 3
        && 0 <= e.mAxis[2] && e.mAxis[2] < 3
        && e.mAxis[1] != e.mAxis[0]
        && e.mAxis[1] != e.mAxis[2])
    {
        if (e.mAxis[0] != e.mAxis[2])
        {
#if defined(GE_USE_MAT_VEC)
            // Map (0,1,2), (1,2,0), and (2,0,1) to +1.
            // Map (0,2,1), (2,1,0), and (1,0,2) to -1.
            int parity = (((e.mAxis[2] | (e.mAxis[1] << 2)) >> e.mAxis[0]) & 1);
            Real const sgn = (parity & 1 ? (Real)-1 : (Real)+1);

            if (rot[e.mAxis[2] * N + e.mAxis[0]] < (Real)1)
            {
                if (rot[e.mAxis[2] * N + e.mAxis[0]] > (Real)-1)
                {
                    e.mAngle[2] = atan2(sgn*rot[e.mAxis[1] * N + e.mAxis[0]],
						rot[e.mAxis[0] * N + e.mAxis[0]]);
                    e.mAngle[1] = asin(-sgn * rot[e.mAxis[2] * N + e.mAxis[0]]);
                    e.mAngle[0] = atan2(
						sgn * rot[e.mAxis[2] * N + e.mAxis[1]],
						rot[e.mAxis[2] * N + e.mAxis[2]]);
                    e.mResult = ER_UNIQUE;
                }
                else
                {
                    e.mAngle[2] = (Real)0;
                    e.mAngle[1] = sgn*(Real)GE_C_HALF_PI;
                    e.mAngle[0] = atan2(
						-sgn * rot[e.mAxis[1] * N + e.mAxis[2]],
						rot[e.mAxis[1] * N + e.mAxis[1]]);
                    e.mResult = ER_NOT_UNIQUE_DIF;
                }
            }
            else
            {
                e.mAngle[2] = (Real)0;
                e.mAngle[1] = -sgn*(Real)GE_C_HALF_PI;
                e.mAngle[0] = atan2(
					-sgn * rot[e.mAxis[1] * N + e.mAxis[2]],
					rot[e.mAxis[1] * N + e.mAxis[1]]);
                e.mResult = ER_NOT_UNIQUE_SUM;
            }
#else
            // Map (0,1,2), (1,2,0), and (2,0,1) to +1.
            // Map (0,2,1), (2,1,0), and (1,0,2) to -1.
            int parity = (((e.mAxis[0] | (e.mAxis[1] << 2)) >> e.mAxis[2]) & 1);
            Real const sgn = (parity & 1 ? (Real)+1 : (Real)-1);

            if (rot[e.mAxis[0] * N + e.mAxis[2]] < (Real)1)
            {
                if (rot[e.mAxis[0] * N + e.mAxis[2]] > (Real)-1)
                {
                    e.mAngle[0] = atan2(sgn*rot[e.mAxis[1] * N + e.mAxis[2]],
						rot[e.mAxis[2] * N + e.mAxis[2]]);
                    e.mAngle[1] = asin(-sgn * rot[e.mAxis[0] * N + e.mAxis[2]]);
                    e.mAngle[2] = atan2(
						sgn * rot[e.mAxis[0] * N + e.mAxis[1]]),
						rot[e.mAxis[0] * N + e.mAxis[0]]);
                    e.mResult = ER_UNIQUE;
                }
                else
                {
                    e.mAngle[0] = (Real)0;
                    e.mAngle[1] = sgn*(Real)GE_C_HALF_PI;
                    e.mAngle[2] = atan2(
						-sgn * rot[e.mAxis[1] * N + e.mAxis[0]]),
						rot[e.mAxis[1] * N + e.mAxis[1]]);
                    e.mResult = ER_NOT_UNIQUE_DIF;
                }
            }
            else
            {
                e.mAngle[0] = (Real)0;
                e.mAngle[1] = -sgn*(Real)GE_C_HALF_PI;
                e.mAngle[2] = atan2(
					-sgn* rot[e.mAxis[1] * N + e.mAxis[0]],
					rot[e.mAxis[1] * N + e.mAxis[1]]);
                e.mResult = ER_NOT_UNIQUE_SUM;
            }
#endif
        }
        else
        {
#if defined(GE_USE_MAT_VEC)
            // Map (0,2,0), (1,0,1), and (2,1,2) to +1.
            // Map (0,1,0), (1,2,1), and (2,0,2) to -1.
            int b0 = 3 - e.mAxis[1] - e.mAxis[2];
            int parity = (((b0 | (e.mAxis[1] << 2)) >> e.mAxis[2]) & 1);
            Real const sgn = (parity & 1 ? (Real)+1 : (Real)-1);

            if (rot[e.mAxis[2] * N + e.mAxis[2]] < (Real)1)
            {
                if (rot[e.mAxis[2] * N + e.mAxis[2]] > (Real)-1)
                {
                    e.mAngle[2] = atan2(
						rot[e.mAxis[1] * N + e.mAxis[2]],
                        sgn * rot[b0 * N + e.mAxis[2]]);
                    e.mAngle[1] = acos(rot[e.mAxis[2] * N + e.mAxis[2]]);
                    e.mAngle[0] = atan2(
						rot[e.mAxis[2] * N + e.mAxis[1]],
                        -sgn * rot[e.mAxis[2] * N + b0]);
                    e.mResult = ER_UNIQUE;
                }
                else
                {
                    e.mAngle[2] = (Real)0;
                    e.mAngle[1] = (Real)GE_C_PI;
                    e.mAngle[0] = atan2(
						sgn*rot[e.mAxis[1] * N + b0],
						rot[e.mAxis[1] * N + e.mAxis[1]]);
                    e.mResult = ER_NOT_UNIQUE_DIF;
                }
            }
            else
            {
                e.mAngle[2] = (Real)0;
                e.mAngle[1] = (Real)0;
                e.mAngle[0] = atan2(
					sgn*rot[e.mAxis[1] * N + b0],
					rot[e.mAxis[1] * N + e.mAxis[1]]);
                e.mResult = ER_NOT_UNIQUE_SUM;
            }
#else
            // Map (0,2,0), (1,0,1), and (2,1,2) to -1.
            // Map (0,1,0), (1,2,1), and (2,0,2) to +1.
            int b2 = 3 - e.mAxis[0] - e.mAxis[1];
            int parity = (((b2 | (e.mAxis[1] << 2)) >> e.mAxis[0]) & 1);
            Real const sgn = (parity & 1 ? (Real)-1 : (Real)+1);

            if (rot[e.mAxis[0] * N + e.mAxis[0]] < (Real)1)
            {
                if (rot[e.mAxis[0] * N + e.mAxis[0]] > (Real)-1)
                {
                    e.mAngle[0] = atan2(
						rot[e.mAxis[1] * N e.mAxis[0]],
                        sgn*rot[b2 * N + e.mAxis[0]]);
                    e.mAngle[1] = acos(rot[e.mAxis[0] * N + e.mAxis[0]]);
                    e.mAngle[2] = atan2(
						rot[e.mAxis[0] * N + e.mAxis[1]],
                        -sgn* rot[e.mAxis[0] * N + b2]);
                    e.mResult = ER_UNIQUE;
                }
                else
                {
                    e.mAngle[0] = (Real)0;
                    e.mAngle[1] = (Real)GE_C_PI;
                    e.mAngle[2] = atan2(
						sgn*rot[e.mAxis[1] * N + b2],
						rot[e.mAxis[1] * N + e.mAxis[1]]);
                    e.mResult = ER_NOT_UNIQUE_DIF;
                }
            }
            else
            {
                e.mAngle[0] = (Real)0;
                e.mAngle[1] = (Real)0;
                e.mAngle[2] = atan2(
					sgn*rot[e.mAxis[1] * N + b2],
					rot[e.mAxis[1] * N + e.mAxis[1]]);
                e.mResult = ER_NOT_UNIQUE_SUM;
            }
#endif
        }
    }
    else
    {
        // Invalid angles.
        e.mAngle[0] = (Real)0;
        e.mAngle[1] = (Real)0;
        e.mAngle[2] = (Real)0;
        e.mResult = ER_INVALID;
    }
}

template <int N, typename Real>
void Rotation<N, Real>::Convert(EulerAngles<Real> const& e, Matrix<N, N, Real>& r)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");

    if (0 <= e.mAxis[0] && e.mAxis[0] < 3
        && 0 <= e.mAxis[1] && e.mAxis[1] < 3
        && 0 <= e.mAxis[2] && e.mAxis[2] < 3
        && e.mAxis[1] != e.mAxis[0]
        && e.mAxis[1] != e.mAxis[2])
    {
        Matrix<N, N, Real> r0, r1, r2;
        Convert(AxisAngle<N, Real>(Vector<N, Real>::Unit(e.mAxis[0]),
            e.mAngle[0]), r0);
        Convert(AxisAngle<N, Real>(Vector<N, Real>::Unit(e.mAxis[1]),
            e.mAngle[1]), r1);
        Convert(AxisAngle<N, Real>(Vector<N, Real>::Unit(e.mAxis[2]),
            e.mAngle[2]), r2);
#if defined(GE_USE_MAT_VEC)
        r = r2*r1*r0;
#else
        r = r0*r1*r2;
#endif
    }
    else
    {
        // Invalid angles.
        r.MakeIdentity();
    }
}

template <int N, typename Real>
void Rotation<N, Real>::Convert(Quaternion<Real> const& q, AxisAngle<N, Real>& a)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");

    a.mAxis.MakeZero();

    Real axisSqrLen = q[0] * q[0] + q[1] * q[1] + q[2] * q[2];
    if (axisSqrLen > (Real)0)
    {
#if defined(GE_USE_MAT_VEC)
        Real adjust = ((Real)1) / sqrt(axisSqrLen);
#else
        Real adjust = ((Real)-1) / sqrt(axisSqrLen);
#endif
        a.mAxis[0] = q[0] * adjust;
        a.mAxis[1] = q[1] * adjust;
        a.mAxis[2] = q[2] * adjust;
        Real cs = eastl::max(eastl::min(q[3], (Real)1), (Real)-1);
        a.mAngle = ((Real)2)*acos(cs);
    }
    else
    {
        // The angle is 0 (modulo 2*pi). Any axis will work, so choose the
        // Unit(0) axis.
        a.mAxis[0] = (Real)1;
        a.mAngle = (Real)0;
    }
}

template <int N, typename Real>
void Rotation<N, Real>::Convert(AxisAngle<N, Real> const& a, Quaternion<Real>& q)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");

#if defined(GE_USE_MAT_VEC)
    Real halfAngle = ((Real)0.5)*a.mAngle;
#else
    Real halfAngle = ((Real)-0.5)*a.mAngle;
#endif
    Real sn = sin(halfAngle);
    q[0] = sn*a.mAxis[0];
    q[1] = sn*a.mAxis[1];
    q[2] = sn*a.mAxis[2];
    q[3] = cos(halfAngle);
}

template <int N, typename Real>
void Rotation<N, Real>::Convert(Quaternion<Real> const& q, EulerAngles<Real>& e)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");

    Matrix<N, N, Real> r;
    Convert(q, r);
    Convert(r, e);
}

template <int N, typename Real>
void Rotation<N, Real>::Convert(EulerAngles<Real> const& e, Quaternion<Real>& q)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");

    Matrix<N, N, Real> r;
    Convert(e, r);
    Convert(r, q);
}

template <int N, typename Real>
void Rotation<N, Real>::Convert(AxisAngle<N, Real> const& a, EulerAngles<Real>& e)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");

    Quaternion<Real> q;
    Convert(a, q);
    Convert(q, e);
}

template <int N, typename Real>
void Rotation<N, Real>::Convert(EulerAngles<Real> const& e, AxisAngle<N, Real>& a)
{
    static_assert(N == 3 || N == 4, "Dimension must be 3 or 4.");

    Quaternion<Real> q;
    Convert(e, q);
    Convert(q, a);
}


#endif