// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef MESHBUFFER_H
#define MESHBUFFER_H

#include "Buffer.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

#include "Graphic/Effect/Material.h"
#include "Mathematic/Surface/Mesh.h"

//! Struct for holding a mesh with a single material.
/** A part of an BaseMesh which has the same material on each face of 
that group. Logical groups of an IMesh need not be put into separate 
mesh buffers, but can be. Separately animated parts of the mesh must 
be put into separate mesh buffers.
Some mesh buffer implementations have limitations on the number of
vertices the buffer can hold. In that case, logical grouping can help.
Moreover, the number of vertices should be optimized for the GPU upload,
which often depends on the type of gfx card. Typial figures are
1000-10000 vertices per buffer.
MeshBuffer is a simple implementation of a MeshBuffer, which supports
up to 65535 vertices.

Since meshbuffers are used for drawing, and hence will be exposed
to the driver, chances are high that they are grab()'ed from somewhere.
It's therefore required to dynamically allocate meshbuffers which are
passed to a video driver and only drop the buffer once it's not used in
the current code block anymore.
*/
class GRAPHIC_ITEM BaseMeshBuffer : public Buffer
{
public:

	//! Get vertices of this meshbuffer
	/** \return Vertice of this buffer. */
	virtual const eastl::shared_ptr<VertexBuffer>& GetVertice() const = 0;

	//! Get indices of this meshbuffer
	/** \return Indice of this buffer. */
	virtual const eastl::shared_ptr<IndexBuffer>& GetIndice() const = 0;

	//! Get the material of this meshbuffer
	/** \return Material of this buffer. */
	virtual const eastl::shared_ptr<Material>& GetMaterial() const = 0;

	//! Get material of this meshbuffer
	/** \return Material of this buffer */
	virtual eastl::shared_ptr<Material>& GetMaterial() = 0;

	virtual inline Vector3<float>& Position(unsigned int i) = 0;
	virtual inline Vector3<float>& Normal(unsigned int i) = 0;
	virtual inline Vector3<float>& Tangent(unsigned int i) = 0;
	virtual inline Vector3<float>& Bitangent(unsigned int i) = 0;
	virtual inline Vector4<float>& Color(unsigned int unit, unsigned int i) = 0;
	virtual inline Vector2<float>& TCoord(unsigned int unit, unsigned int i) = 0;
};


class GRAPHIC_ITEM MeshBuffer : public BaseMeshBuffer
{
public:

	MeshBuffer();
	MeshBuffer(VertexFormat const& vformat, uint32_t numVertices,
		uint32_t numPrimitives, size_t indexSize);

	~MeshBuffer();

	//! Get vertices of this meshbuffer
	/** \return Vertice of this buffer. */
	virtual const eastl::shared_ptr<VertexBuffer>& GetVertice() const
	{
		return mVertice;
	}

	//! Get indices of this meshbuffer
	/** \return Indice of this buffer. */
	virtual const eastl::shared_ptr<IndexBuffer>& GetIndice() const
	{
		return mIndice;
	}

	//! Get the material of this meshbuffer
	/** \return Material of this buffer. */
	virtual const eastl::shared_ptr<Material>& GetMaterial() const
	{
		return mMaterial;
	}

	//! Get material of this meshbuffer
	/** \return Material of this buffer */
	virtual eastl::shared_ptr<Material>& GetMaterial()
	{
		return mMaterial;
	}

	virtual inline Vector3<float>& Position(unsigned int i);
	virtual inline Vector3<float>& Normal(unsigned int i);
	virtual inline Vector3<float>& Tangent(unsigned int i);
	virtual inline Vector3<float>& Bitangent(unsigned int i);
	virtual inline Vector4<float>& Color(unsigned int unit, unsigned int i);
	virtual inline Vector2<float>& TCoord(unsigned int unit, unsigned int i);

protected:
	//! indices for this meshbuffer.
	eastl::shared_ptr<IndexBuffer> mIndice;

	//! vertices for this meshbuffer.
	eastl::shared_ptr<VertexBuffer> mVertice;

	//! Material for this meshbuffer.
	eastl::shared_ptr<Material> mMaterial;

	// Support for creating vertex and index buffers.
	eastl::shared_ptr<VertexBuffer> CreateVBuffer(unsigned int numVertices);
	eastl::shared_ptr<IndexBuffer> CreateIBuffer(unsigned int numTriangles, size_t indexSize);

	// Support for vertex buffers.
	char* GetGeometricChannel(eastl::shared_ptr<VertexBuffer> const& vbuffer,
		VASemantic semantic, float w);

	void SetPosition(unsigned int i, Vector3<float> const& pos);
	void SetNormal(unsigned int i, Vector3<float> const& nor);
	void SetTangent(unsigned int i, Vector3<float> const& tan);
	void SetBinormal(unsigned int i, Vector3<float> const& bin);
	void SetColor(unsigned int i, Vector4<float> const& cl);
	void SetTCoord(unsigned int i, Vector2<float> const& tcd);

	// Support for index buffers.
	void ReverseTriangleOrder(IndexBuffer* ibuffer);

	VertexFormat mVFormat;
	bool mAssignTCoords[VA_MAX_TCOORD_UNITS];
	bool mAssignColors[VA_MAX_COLOR_UNITS];

	char* mPositions;
	char* mNormals;
	char* mTangents;
	char* mBitangents;
	char* mColors[VA_MAX_COLOR_UNITS];
	char* mTCoords[VA_MAX_TCOORD_UNITS];
};


inline Vector3<float>& MeshBuffer::Position(unsigned int i)
{
	return *reinterpret_cast<Vector3<float>*>(
		mPositions + i * mVFormat.GetVertexSize());
}

inline Vector3<float>& MeshBuffer::Normal(unsigned int i)
{
	return *reinterpret_cast<Vector3<float>*>(
		mNormals + i * mVFormat.GetVertexSize());
}

inline Vector3<float>& MeshBuffer::Tangent(unsigned int i)
{
	return *reinterpret_cast<Vector3<float>*>(
		mTangents + i * mVFormat.GetVertexSize());
}

inline Vector3<float>& MeshBuffer::Bitangent(unsigned int i)
{
	return *reinterpret_cast<Vector3<float>*>(
		mBitangents + i * mVFormat.GetVertexSize());
}

inline Vector4<float>& MeshBuffer::Color(unsigned int unit, unsigned int i)
{
	return *reinterpret_cast<Vector4<float>*>(
		mColors[unit] + i * mVFormat.GetVertexSize());
}

inline Vector2<float>& MeshBuffer::TCoord(unsigned int unit, unsigned int i)
{
	return *reinterpret_cast<Vector2<float>*>(
		mTCoords[unit] + i * mVFormat.GetVertexSize());
}

#endif