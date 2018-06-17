// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef EULERANGLES_H
#define EULERANGLES_H

#include "Vector.h"

// Factorization into Euler angles is not necessarily unique.  Let the
// integer indices for the axes be (N0,N1,N2), which must be in the set
//   {(0,1,2),(0,2,1),(1,0,2),(1,2,0),(2,0,1),(2,1,0),
//    (0,1,0),(0,2,0),(1,0,1),(1,2,1),(2,0,2),(2,1,2)}
// Let the corresponding angles be (angleN0,angleN1,angleN2).  If the
// result is ER_NOT_UNIQUE_SUM, then the multiple solutions occur because
// angleN2+angleN0 is constant.  If the result is ER_NOT_UNIQUE_DIF, then
// the multiple solutions occur because angleN2-angleN0 is constant.  In
// either type of nonuniqueness, the function returns angleN0=0.
enum EulerResult
{
    // The solution is invalid (incorrect axis indices).
    ER_INVALID,

    // The solution is unique.
    ER_UNIQUE,

    // The solution is not unique.  A sum of angles is constant.
    ER_NOT_UNIQUE_SUM,

    // The solution is not unique.  A difference of angles is constant.
    ER_NOT_UNIQUE_DIF
};

template <typename Real>
class EulerAngles
{
public:
    EulerAngles();
    EulerAngles(int i0, int i1, int i2, Real a0, Real a1, Real a2);

    int mAxis[3];
    Real mAngle[3];

    // This member is set during conversions from rotation matrices,
    // quaternions, or axis-angles.
    EulerResult mResult;
};


template <typename Real>
EulerAngles<Real>::EulerAngles()
    :
    mResult(ER_INVALID)
{
    for (int i = 0; i < 3; ++i)
    {
        mAxis[i] = 0;
        mAngle[i] = (Real)0;
    }
}

template <typename Real>
EulerAngles<Real>::EulerAngles(int i0, int i1, int i2, Real a0, Real a1, Real a2)
    :
    mResult(ER_UNIQUE)
{
    mAxis[0] = i0;
    mAxis[1] = i1;
    mAxis[2] = i2;
    mAngle[0] = a0;
    mAngle[1] = a1;
    mAngle[2] = a2;
}


#endif