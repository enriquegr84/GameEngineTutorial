// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.2 (2016/08/29)

#ifndef MESHFACTORY_H
#define MESHFACTORY_H

#include "Mathematic/Surface/Mesh.h"

#include "Graphic/Resource/Buffer/IndexBuffer.h"
#include "Graphic/Resource/Buffer/VertexBuffer.h"

#include "Graphic/Scene/Hierarchy/Visual.h"

// This class is a factory for Visual objects corresponding to common
// geometric primitives.  Triangle mesh primitives are generated.  Each mesh
// is centered at (0,0,0) and has an up-axis of (0,0,1).  The other axes
// forming the coordinate system are (1,0,0) and (0,1,0).
//
// The factory always generates 3-tuple positions.  If normals, tangents, or
// binormals are requested, they are also generated as 3-tuples.  They are
// stored in the vertex buffer as 3-tuples or 4-tuples as requested (w = 1 for
// positions, w = 0 for the others).  The factory also generates 2-tuple
// texture coordinates.  These are stored in the vertex buffer for 2-tuple
// units.  All other attribute types are unassigned by the factory.

class GRAPHIC_ITEM MeshFactory
{
public:
    // Construction and destruction.
    ~MeshFactory();
    MeshFactory();

    // Specify the vertex format.
    inline void SetVertexFormat(VertexFormat const& format);

    // Specify the usage for the vertex buffer data.  The default is
    // Resource::IMMUTABLE.
    inline void SetVertexBufferUsage(Resource::Usage usage);

    // Specify the type of indices and where the index buffer data should be
    // stored.  For 'unsigned int' indices, set 'use32Bit' to 'true'; for
    // 'unsigned short' indices, set 'use32Bit' to false.  The default is
    // 'unsigned int'.
    inline void SetIndexFormat(bool use32Bit);
    
    // Specify the usage for the index buffer data.  The default is
    // Resource::IMMUTABLE.
    inline void SetIndexBufferUsage(Resource::Usage usage);

    // For the geometric primitives that have an inside and an outside, you
    // may specify where the observer is expected to see the object.  If the
    // observer must see the primitive from the outside, pass 'true' to this
    // function.  If the observer must see the primitive from the inside, pass
    // 'false'.  This Boolean flag simply controls the triangle face order
    // for face culling.  The default is 'true' (observer view object from the
    // outside).
    inline void SetOutside(bool outside);

    // The triangle is in the plane z = 0 and is visible to an observer who is
    // on the side of the plane to which the normal (0,0,1) points.  It has
    // vertices (0, 0, 0), (xExtent, 0, 0), and (0, yExtent, 0).  The mesh
    // has numSamples vertices along each of the x- and y-axes for a total
    // of numSamples*(numSamples+1)/2 vertices.
	eastl::shared_ptr<Visual> CreateTriangle(unsigned int numSamples,
        float xExtent, float yExtent);

private:
    // Support for creating vertex and index buffers.
	eastl::shared_ptr<VertexBuffer> CreateVBuffer(unsigned int numVertices);
	eastl::shared_ptr<IndexBuffer> CreateIBuffer(unsigned int numTriangles);

    // Support for vertex buffers.
    char* GetGeometricChannel(eastl::shared_ptr<VertexBuffer> const& vbuffer,
        VASemantic semantic, float w);
    inline Vector3<float>& Position(unsigned int i);
    inline Vector3<float>& Normal(unsigned int i);
    inline Vector3<float>& Tangent(unsigned int i);
    inline Vector3<float>& Bitangent(unsigned int i);
    inline Vector2<float>& TCoord(unsigned int unit, unsigned int i);
    void SetPosition(unsigned int i, Vector3<float> const& pos);
    void SetNormal(unsigned int i, Vector3<float> const& nor);
    void SetTangent(unsigned int i, Vector3<float> const& tan);
    void SetBinormal(unsigned int i, Vector3<float> const& bin);
    void SetTCoord(unsigned int i, Vector2<float> const& tcd);

    VertexFormat mVFormat;
    size_t mIndexSize;
    Resource::Usage mVBUsage, mIBUsage;
    bool mOutside;
    bool mAssignTCoords[VA_MAX_TCOORD_UNITS];

    char* mPositions;
    char* mNormals;
    char* mTangents;
    char* mBitangents;
    char* mTCoords[VA_MAX_TCOORD_UNITS];
};


inline void MeshFactory::SetVertexFormat(VertexFormat const& format)
{
    mVFormat = format;
}

inline void MeshFactory::SetVertexBufferUsage(Resource::Usage usage)
{
    mVBUsage = usage;
}

inline void MeshFactory::SetIndexFormat(bool use32Bit)
{
    mIndexSize = (use32Bit ? sizeof(unsigned int) : sizeof(unsigned short));
}

inline void MeshFactory::SetIndexBufferUsage(Resource::Usage usage)
{
    mIBUsage = usage;
}

inline void MeshFactory::SetOutside(bool outside)
{
    mOutside = outside;
}

inline Vector3<float>& MeshFactory::Position(unsigned int i)
{
    return *reinterpret_cast<Vector3<float>*>(
        mPositions + i*mVFormat.GetVertexSize());
}

inline Vector3<float>& MeshFactory::Normal(unsigned int i)
{
    return *reinterpret_cast<Vector3<float>*>(
        mNormals + i*mVFormat.GetVertexSize());
}

inline Vector3<float>& MeshFactory::Tangent(unsigned int i)
{
    return *reinterpret_cast<Vector3<float>*>(
        mTangents + i*mVFormat.GetVertexSize());
}

inline Vector3<float>& MeshFactory::Bitangent(unsigned int i)
{
    return *reinterpret_cast<Vector3<float>*>(
        mBitangents + i*mVFormat.GetVertexSize());
}

inline Vector2<float>& MeshFactory::TCoord(unsigned int unit, unsigned int i)
{
    return *reinterpret_cast<Vector2<float>*>(
        mTCoords[unit] + i*mVFormat.GetVertexSize());
}

#endif