// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/07/28)

#ifndef HYPERPLANE_H
#define HYPERPLANE_H

#include "Mathematic/Algebra/Vector.h"
#include "Mathematic/Function/Constants.h"
#include "Mathematic/NumericalMethod/SingularValueDecomposition.h"

// The plane is represented as Dot(U,X) = c where U is a unit-length normal
// vector, c is the plane constant, and X is any point on the plane.  The user
// must ensure that the normal vector is unit length.

template <int N, typename Real>
class Hyperplane
{
public:
    // Construction and destruction.  The default constructor sets the normal
    // to (0,...,0,1) and the constant to zero (plane z = 0).
    Hyperplane();

    // Specify U and c directly.
    Hyperplane(Vector<N, Real> const& inNormal, Real inConstant);

    // U is specified, c = Dot(U,p) where p is a point on the hyperplane.
    Hyperplane(Vector<N, Real> const& inNormal, Vector<N, Real> const& p);

	// U is a unit-length vector in the orthogonal complement of the set
	// {p[1]-p[0],...,p[n-1]-p[0]} and c = Dot(U,p[0]), where the p[i] are
	// pointson the hyperplane.
	Hyperplane(eastl::array<Vector<N, Real>, N> const& p);

    // Public member access.
    Vector<N, Real> mNormal;
    Real mConstant;

public:

    // Comparisons to support sorted containers.
    bool operator==(Hyperplane const& hyperplane) const;
    bool operator!=(Hyperplane const& hyperplane) const;
    bool operator< (Hyperplane const& hyperplane) const;
    bool operator<=(Hyperplane const& hyperplane) const;
    bool operator> (Hyperplane const& hyperplane) const;
    bool operator>=(Hyperplane const& hyperplane) const;
};

// Template alias for convenience.
template <typename Real>
using Plane3 = Hyperplane<3, Real>;

template <int N, typename Real>
Hyperplane<N, Real>::Hyperplane()
    :
    mConstant((Real)0)
{
    mNormal.MakeUnit(N - 1);
}

template <int N, typename Real>
Hyperplane<N, Real>::Hyperplane(Vector<N, Real> const& inNormal,
    Real inConstant)
    :
    mNormal(inNormal),
    mConstant(inConstant)
{
}

template <int N, typename Real>
Hyperplane<N, Real>::Hyperplane(Vector<N, Real> const& inNormal,
    Vector<N, Real> const& p)
    :
    mNormal(inNormal),
    mConstant(Dot(inNormal, p))
{
}


template <int N, typename Real>
Hyperplane<N, Real>::Hyperplane(eastl::array<Vector<N, Real>, N> const& p)
{
	Matrix<N, N - 1, Real> edge;
	for (int i = 0; i < N - 1; ++i)
	{
		edge.SetCol(i, p[i + 1] - p[0]);
	}

	// Compute the 1-dimensional orthogonal complement of the edges of the
	// simplex formed by the points p[].
	SingularValueDecomposition<Real> svd(N, N - 1, 32);
	svd.Solve(&edge[0], -1);
	svd.GetUColumn(N - 1, &mNormal[0]);

	mConstant = Dot(mNormal, p[0]);
}

template <int N, typename Real>
bool Hyperplane<N, Real>::operator==(Hyperplane const& hyperplane) const
{
    return mNormal == hyperplane.mNormal && mConstant == hyperplane.mConstant;
}

template <int N, typename Real>
bool Hyperplane<N, Real>::operator!=(Hyperplane const& hyperplane) const
{
    return !operator==(hyperplane);
}

template <int N, typename Real>
bool Hyperplane<N, Real>::operator<(Hyperplane const& hyperplane) const
{
    if (mNormal < hyperplane.mNormal)
    {
        return true;
    }

    if (mNormal > hyperplane.mNormal)
    {
        return false;
    }

    return mConstant < hyperplane.mConstant;
}

template <int N, typename Real>
bool Hyperplane<N, Real>::operator<=(Hyperplane const& hyperplane) const
{
    return operator<(hyperplane) || operator==(hyperplane);
}

template <int N, typename Real>
bool Hyperplane<N, Real>::operator>(Hyperplane const& hyperplane) const
{
    return !operator<=(hyperplane);
}

template <int N, typename Real>
bool Hyperplane<N, Real>::operator>=(Hyperplane const& hyperplane) const
{
    return !operator<(hyperplane);
}


#endif