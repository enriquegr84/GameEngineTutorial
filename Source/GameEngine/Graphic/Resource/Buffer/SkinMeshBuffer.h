// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef SKINMESHBUFFER_H
#define SKINMESHBUFFER_H

#include "Buffer.h"
#include "MeshBuffer.h"

#include "Graphic/Effect/Material.h"
#include "Mathematic/Surface/Mesh.h"
#include "Mathematic/Algebra/Transform.h"

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

//! A mesh buffer able to choose between VertexTCoords, Vertex and VertexTangents at runtime
class GRAPHIC_ITEM SkinMeshBuffer : public Buffer
{
public:

	SkinMeshBuffer();
	SkinMeshBuffer(VertexFormat const& vformat, uint32_t numVertices,
		uint32_t numPrimitives, size_t indexSize);

	~SkinMeshBuffer();

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

	Transform& GetTransform()
	{
		return mTransform;
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
	eastl::shared_ptr<VertexBuffer> mVerticeTangent;
	eastl::shared_ptr<VertexBuffer> mVerticeTCoords;

	VertexType mVertexType;

	Transform mTransform;

	//! Material for this meshbuffer.
	eastl::shared_ptr<Material> mMaterial;

	// Support for creating vertex and index buffers.
	void CreateVBuffer(unsigned int numVertices);
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

	char* mVertexPositions;
	char* mVertexNormals;
	char* mVertexTangents;
	char* mVertexBitangents;
	char* mVertexColors[VA_MAX_COLOR_UNITS];
	char* mVertexTCoords[VA_MAX_TCOORD_UNITS];

	char* mVertexTanPositions;
	char* mVertexTanNormals;
	char* mVertexTanTangents;
	char* mVertexTanBitangents;
	char* mVertexTanColors[VA_MAX_COLOR_UNITS];
	char* mVertexTanTCoords[VA_MAX_TCOORD_UNITS];

	char* mVertexTCPositions;
	char* mVertexTCNormals;
	char* mVertexTCTangents;
	char* mVertexTCBitangents;
	char* mVertexTCColors[VA_MAX_COLOR_UNITS];
	char* mVertexTCTCoords[VA_MAX_TCOORD_UNITS];
};


inline Vector3<float>& SkinMeshBuffer::Position(unsigned int i)
{
	switch (mVertexType)
	{
		case VT_2TCOORDS:
			return *reinterpret_cast<Vector3<float>*>(
				mVertexTCPositions + i * mVFormat.GetVertexSize());
			break;
		case VT_TANGENTS:
			return *reinterpret_cast<Vector3<float>*>(
				mVertexTanPositions + i * mVFormat.GetVertexSize());
			break;
		case VT_STANDARD:
			return *reinterpret_cast<Vector3<float>*>(
				mVertexPositions + i * mVFormat.GetVertexSize());
			break;
	}
}

inline Vector3<float>& SkinMeshBuffer::Normal(unsigned int i)
{
	switch (mVertexType)
	{
		case VT_2TCOORDS:
			return *reinterpret_cast<Vector3<float>*>(
				mVertexTCNormals + i * mVFormat.GetVertexSize());
			break;
		case VT_TANGENTS:
			return *reinterpret_cast<Vector3<float>*>(
				mVertexTanNormals + i * mVFormat.GetVertexSize());
			break;
		case VT_STANDARD:
			return *reinterpret_cast<Vector3<float>*>(
				mVertexNormals + i * mVFormat.GetVertexSize());
			break;
	}
}

inline Vector3<float>& SkinMeshBuffer::Tangent(unsigned int i)
{
	switch (mVertexType)
	{
		case VT_2TCOORDS:
			return *reinterpret_cast<Vector3<float>*>(
				mVertexTCTangents + i * mVFormat.GetVertexSize());
			break;
		case VT_TANGENTS:
			return *reinterpret_cast<Vector3<float>*>(
				mVertexTanTangents + i * mVFormat.GetVertexSize());
			break;
		case VT_STANDARD:
			return *reinterpret_cast<Vector3<float>*>(
				mVertexTangents + i * mVFormat.GetVertexSize());
			break;
	}
}

inline Vector3<float>& SkinMeshBuffer::Bitangent(unsigned int i)
{
	switch (mVertexType)
	{
		case VT_2TCOORDS:
			return *reinterpret_cast<Vector3<float>*>(
				mVertexTCBitangents + i * mVFormat.GetVertexSize());
			break;
		case VT_TANGENTS:
			return *reinterpret_cast<Vector3<float>*>(
				mVertexTanBitangents + i * mVFormat.GetVertexSize());
			break;
		case VT_STANDARD:
			return *reinterpret_cast<Vector3<float>*>(
				mVertexBitangents + i * mVFormat.GetVertexSize());
			break;
	}
}

inline Vector4<float>& SkinMeshBuffer::Color(unsigned int unit, unsigned int i)
{
	switch (mVertexType)
	{
		case VT_2TCOORDS:
			return *reinterpret_cast<Vector4<float>*>(
				mVertexTCColors[unit] + i * mVFormat.GetVertexSize());
			break;
		case VT_TANGENTS:
			return *reinterpret_cast<Vector4<float>*>(
				mVertexTanColors[unit] + i * mVFormat.GetVertexSize());
			break;
		case VT_STANDARD:
			return *reinterpret_cast<Vector4<float>*>(
				mVertexColors[unit] + i * mVFormat.GetVertexSize());
			break;
	}
}

inline Vector2<float>& SkinMeshBuffer::TCoord(unsigned int unit, unsigned int i)
{
	switch (mVertexType)
	{
		case VT_2TCOORDS:
			return *reinterpret_cast<Vector2<float>*>(
				mVertexTCTCoords[unit] + i * mVFormat.GetVertexSize());
			break;
		case VT_TANGENTS:
			return *reinterpret_cast<Vector2<float>*>(
				mVertexTanTCoords[unit] + i * mVFormat.GetVertexSize());
			break;
		case VT_STANDARD:
			return *reinterpret_cast<Vector2<float>*>(
				mVertexTCoords[unit] + i * mVFormat.GetVertexSize());
			break;
	}
}

#endif