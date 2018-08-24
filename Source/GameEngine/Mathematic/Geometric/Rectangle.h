// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "Mathematic/Algebra/Vector.h"

// Points are R(s0,s1) = C + s0*A0 + s1*A1, where C is the center of the
// rectangle and A0 and A1 are unit-length and perpendicular axes.  The
// parameters s0 and s1 are constrained by |s0| <= e0 and |s1| <= e1,
// where e0 > 0 and e1 > 0 are the extents of the rectangle.
template <int N, typename Real>
class RectangleShape
{
public:
    // Construction and destruction.  The default constructor sets the origin
    // to (0,...,0), axis A0 to (1,0,...,0), axis A1 to (0,1,0,...0), and both
    // extents to 1.
    RectangleShape();
    RectangleShape(Vector<N, Real> const& inCenter,
        eastl::array<Vector<N, Real>, 2> const& inAxis,
        Vector<2, Real> const& inExtent);

	bool IsPointInside(const Vector2<Real>& point) const;

    // Compute the vertices of the rectangle.  If index i has the bit pattern
    // i = b[1]b[0], then
    //   vertex[i] = center + sum_{d=0}^{1} sign[d] * extent[d] * axis[d]
    // where sign[d] = 2*b[d] - 1.
    void GetVertices(eastl::array<Vector<N, Real>, 4>& vertex) const;

    Vector<N, Real> mCenter;
	eastl::array<Vector<N, Real>, 2> mAxis;
    Vector<2, Real> mExtent;

public:
    // Comparisons to support sorted containers.
    bool operator==(RectangleShape const& rectangle) const;
    bool operator!=(RectangleShape const& rectangle) const;
    bool operator< (RectangleShape const& rectangle) const;
    bool operator<=(RectangleShape const& rectangle) const;
    bool operator> (RectangleShape const& rectangle) const;
    bool operator>=(RectangleShape const& rectangle) const;
};

// Template alias for convenience.
template <typename Real>
using Rectangle3 = RectangleShape<3, Real>;


template <int N, typename Real>
RectangleShape<N, Real>::RectangleShape()
{
    mCenter.MakeZero();
    for (int i = 0; i < 2; ++i)
    {
        mAxis[i].MakeUnit(i);
        mExtent[i] = (Real)1;
    }
}

template <int N, typename Real>
RectangleShape<N, Real>::RectangleShape(Vector<N, Real> const& inCenter,
	eastl::array<Vector<N, Real>, 2> const& inAxis,
    Vector<2, Real> const& inExtent)
    :
    mCenter(inCenter),
    mAxis(inAxis),
    mExtent(inExtent)
{
}

template <int N, typename Real>
bool RectangleShape<N, Real>::IsPointInside(const Vector2<Real>& point) const
{
	return (
		mCenter[0] - (mExtent[0] / 2) <= point[0] &&
		mCenter[1] - (mExtent[1] / 2) <= point[1] &&
		mCenter[0] + (int)round(mExtent[0] / 2.f) >= point[0] &&
		mCenter[1] + (int)round(mExtent[1] / 2.f) >= point[1]);
}

template <int N, typename Real>
void RectangleShape<N, Real>::GetVertices(
	eastl::array<Vector<N, Real>, 4>& vertex) const
{
    Vector<N, Real> product0 = mExtent[0] * mAxis[0];
    Vector<N, Real> product1 = mExtent[1] * mAxis[1];
    Vector<N, Real> sum = product0 + product1;
    Vector<N, Real> dif = product0 - product1;

    vertex[0] = mCenter - sum;
    vertex[1] = mCenter + dif;
    vertex[2] = mCenter - dif;
    vertex[3] = mCenter + sum;
}

template <int N, typename Real>
bool RectangleShape<N, Real>::operator==(RectangleShape const& rectangle) const
{
    if (mCenter != rectangle.mCenter)
    {
        return false;
    }

    for (int i = 0; i < 2; ++i)
    {
        if (mAxis[i] != rectangle.mAxis[i])
        {
            return false;
        }
    }

    for (int i = 0; i < 2; ++i)
    {
        if (mExtent[i] != rectangle.mExtent[i])
        {
            return false;
        }
    }

    return true;
}

template <int N, typename Real>
bool RectangleShape<N, Real>::operator!=(RectangleShape const& rectangle) const
{
    return !operator==(rectangle);
}

template <int N, typename Real>
bool RectangleShape<N, Real>::operator<(RectangleShape const& rectangle) const
{
    if (mCenter < rectangle.mCenter)
    {
        return true;
    }

    if (mCenter > rectangle.mCenter)
    {
        return false;
    }

    if (mAxis < rectangle.mAxis)
    {
        return true;
    }

    if (mAxis > rectangle.mAxis)
    {
        return false;
    }

    return mExtent < rectangle.mExtent;
}

template <int N, typename Real>
bool RectangleShape<N, Real>::operator<=(RectangleShape const& rectangle) const
{
    return operator<(rectangle) || operator==(rectangle);
}

template <int N, typename Real>
bool RectangleShape<N, Real>::operator>(RectangleShape const& rectangle) const
{
    return !operator<=(rectangle);
}

template <int N, typename Real>
bool RectangleShape<N, Real>::operator>=(RectangleShape const& rectangle) const
{
    return !operator<(rectangle);
}

#endif