// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.4 (2016/08/29)

#ifndef RECTANGLEMESH_H
#define RECTANGLEMESH_H

#include "Mesh.h"

#include "Mathematic/Geometric/Rectangle.h"
#include "Mathematic/Algebra/Vector3.h"


template <typename Real>
class RectangleMesh : public Mesh<Real>
{
public:
    // Create a mesh that tessellates a rectangle.
    RectangleMesh(MeshDescription const& description, RectangleShape<3, Real> const& rectangle);

    // Member access.
    inline RectangleShape<3, Real> const& GetRectangle() const;

protected:
    void InitializeTCoords();
    void InitializePositions();
    void InitializeNormals();
    void InitializeFrame();

    RectangleShape<3, Real> mRectangle;

    // If the client does not request texture coordinates, they will be
    // computed internally for use in evaluation of the surface geometry.
    eastl::vector<Vector2<Real>> mDefaultTCoords;
};


template <typename Real>
RectangleMesh<Real>::RectangleMesh(MeshDescription const& description,
    RectangleShape<3, Real> const& rectangle)
    :
    Mesh<Real>(description, { MeshTopology::RECTANGLE }),
    mRectangle(rectangle)
{
    if (!this->mDescription.mConstructed)
    {
        // The logger system will report these errors in the Mesh constructor.
        return;
    }

    if (!this->mTCoords)
    {
        mDefaultTCoords.resize(this->mDescription.mNumVertices);
        this->mTCoords = mDefaultTCoords.data();
        this->mTCoordStride = sizeof(Vector2<Real>);

        this->mDescription.mAllowUpdateFrame = this->mDescription.mWantDynamicTangentSpaceUpdate;
        if (this->mDescription.mAllowUpdateFrame)
        {
            if (!this->mDescription.mHasTangentSpaceVectors)
            {
                this->mDescription.mAllowUpdateFrame = false;
            }

            if (!this->mNormals)
            {
                this->mDescription.mAllowUpdateFrame = false;
            }
        }
    }

    this->ComputeIndices();
    InitializeTCoords();
    InitializePositions();
    if (this->mDescription.mAllowUpdateFrame)
    {
        InitializeFrame();
    }
    else if (this->mNormals)
    {
        InitializeNormals();
    }
}

template <typename Real>
inline RectangleShape<3, Real> const& RectangleMesh<Real>::GetRectangle() const
{
    return mRectangle;
}

template <typename Real>
void RectangleMesh<Real>::InitializeTCoords()
{
    Vector2<Real> tcoord;
    for (uint32_t r = 0, i = 0; r < this->mDescription.mNumRows; ++r)
    {
        tcoord[1] = (Real)r / (Real)(this->mDescription.mNumRows - 1);
        for (uint32_t c = 0; c < this->mDescription.mNumCols; ++c, ++i)
        {
            tcoord[0] = (Real)c / (Real)(this->mDescription.mNumCols - 1);
            this->TCoord(i) = tcoord;
        }
    }
}

template <typename Real>
void RectangleMesh<Real>::InitializePositions()
{
    for (uint32_t r = 0, i = 0; r < this->mDescription.mNumRows; ++r)
    {
        for (uint32_t c = 0; c < this->mDescription.mNumCols; ++c, ++i)
        {
            Vector2<Real> tcoord = this->TCoord(i);
            Real w0 = ((Real)2 * tcoord[0] - (Real)1) * mRectangle.mExtent[0];
            Real w1 = ((Real)2 * tcoord[1] - (Real)1) * mRectangle.mExtent[1];
            this->Position(i) = mRectangle.mCenter + w0 * mRectangle.mAxis[0] + w1 * mRectangle.mAxis[1];
        }
    }
}

template <typename Real>
void RectangleMesh<Real>::InitializeNormals()
{
    Vector3<Real> normal = UnitCross(mRectangle.mAxis[0], mRectangle.mAxis[1]);
    for (uint32_t i = 0; i < this->mDescription.mNumVertices; ++i)
    {
        this->Normal(i) = normal;
    }
}

template <typename Real>
void RectangleMesh<Real>::InitializeFrame()
{
    Vector3<Real> normal = UnitCross(mRectangle.mAxis[0], mRectangle.mAxis[1]);
    Vector3<Real> tangent{ (Real)1, (Real)0, (Real)0 };
    Vector3<Real> bitangent{ (Real)0, (Real)1, (Real)0 };  // = Cross(normal,tangent)
    for (uint32_t i = 0; i < this->mDescription.mNumVertices; ++i)
    {
        if (this->mNormals)
        {
            this->Normal(i) = normal;
        }

        if (this->mTangents)
        {
            this->Tangent(i) = tangent;
        }

        if (this->mBitangents)
        {
            this->Bitangent(i) = bitangent;
        }

        if (this->mDPDUs)
        {
            this->DPDU(i) = tangent;
        }

        if (this->mDPDVs)
        {
            this->DPDV(i) = bitangent;
        }
    }
}

#endif