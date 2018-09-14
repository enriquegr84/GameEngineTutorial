// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "MeshBuffer.h"

MeshBuffer::MeshBuffer(VertexFormat const& vformat, uint32_t numVertices,
	uint32_t numPrimitives, size_t indexSize)
	: Buffer(numPrimitives, 0)
{
	mVFormat = vformat;

	mIndice = CreateIBuffer(numPrimitives, indexSize);
	if (!mIndice)
		LogError("Error creating index buffer");
	mVertice = CreateVBuffer(numVertices);
	if (!mVertice)
		LogError("Error creating vertex buffer");

	mMaterial = eastl::make_shared<Material>(new Material());
}

MeshBuffer::~MeshBuffer()
{

}

eastl::shared_ptr<VertexBuffer> MeshBuffer::CreateVBuffer(unsigned int numVertices)
{
	auto vbuffer = eastl::make_shared<VertexBuffer>(mVFormat, numVertices);
	if (vbuffer)
	{
		// Get the position channel.
		mPositions = GetGeometricChannel(vbuffer, VA_POSITION, 1.0f);
		if (!mPositions)
		{
			LogError("Positions are required.");
			return nullptr;
		}

		// Get the optional geometric channels.
		mNormals = GetGeometricChannel(vbuffer, VA_NORMAL, 0.0f);
		mTangents = GetGeometricChannel(vbuffer, VA_TANGENT, 0.0f);
		mBitangents = GetGeometricChannel(vbuffer, VA_BINORMAL, 0.0f);

		// Get color channels that are to be assigned values.
		// Clear the mAssignColors element in case any elements were set by a
		// previous creation call.
		eastl::set<DFType> required;
		required.insert(DF_R32G32B32A32_FLOAT);
		for (unsigned int unit = 0; unit < VA_MAX_COLOR_UNITS; ++unit)
		{
			mColors[unit] = vbuffer->GetChannel(VA_COLOR, unit, required);
			if (mColors[unit])
			{
				mAssignColors[unit] = true;
			}
			else
			{
				mAssignColors[unit] = false;
			}
		}

		// Get texture coordinate channels that are to be assigned values.
		// Clear the mAssignTCoords element in case any elements were set by a
		// previous creation call.
		required.clear();
		required.insert(DF_R32G32_FLOAT);
		for (unsigned int unit = 0; unit < VA_MAX_TCOORD_UNITS; ++unit)
		{
			mTCoords[unit] = vbuffer->GetChannel(VA_TEXCOORD, unit, required);
			if (mTCoords[unit])
			{
				mAssignTCoords[unit] = true;
			}
			else
			{
				mAssignTCoords[unit] = false;
			}
		}

		vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
	}
	return vbuffer;
}

eastl::shared_ptr<IndexBuffer> MeshBuffer::CreateIBuffer(unsigned int numTriangles, size_t indexSize)
{
	auto ibuffer = eastl::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, indexSize);
	if (ibuffer)
	{
		ibuffer->SetUsage(Resource::IMMUTABLE);
	}
	return ibuffer;
}

char* MeshBuffer::GetGeometricChannel(
	eastl::shared_ptr<VertexBuffer> const& vbuffer, VASemantic semantic, float w)
{
	char* channel = nullptr;
	int index = mVFormat.GetIndex(semantic, 0);
	if (index >= 0)
	{
		channel = vbuffer->GetChannel(semantic, 0, eastl::set<DFType>());
		LogAssert(channel != 0, "Unexpected condition.");
		if (mVFormat.GetType(index) == DF_R32G32B32A32_FLOAT)
		{
			// Fill in the w-components.
			int const numVertices = vbuffer->GetNumElements();
			for (int i = 0; i < numVertices; ++i)
			{
				float* tuple4 = reinterpret_cast<float*>(
					channel + i * mVFormat.GetVertexSize());
				tuple4[3] = w;
			}
		}
	}
	return channel;
}

void MeshBuffer::SetPosition(unsigned int i, Vector3<float> const& pos)
{
	Position(i) = pos;
}

void MeshBuffer::SetNormal(unsigned int i, Vector3<float> const& nor)
{
	if (mNormals)
	{
		Normal(i) = nor;
	}
}

void MeshBuffer::SetTangent(unsigned int i, Vector3<float> const& tan)
{
	if (mTangents)
	{
		Tangent(i) = tan;
	}
}

void MeshBuffer::SetBinormal(unsigned int i, Vector3<float> const& bin)
{
	if (mBitangents)
	{
		Bitangent(i) = bin;
	}
}

void MeshBuffer::SetColor(unsigned int i, Vector4<float> const& cl)
{
	for (unsigned int unit = 0; unit < VA_MAX_COLOR_UNITS; ++unit)
	{
		if (mAssignColors[unit])
		{
			Color(unit, i) = cl;
		}
	}
}

void MeshBuffer::SetTCoord(unsigned int i, Vector2<float> const& tcd)
{
	for (unsigned int unit = 0; unit < VA_MAX_TCOORD_UNITS; ++unit)
	{
		if (mAssignTCoords[unit])
		{
			TCoord(unit, i) = tcd;
		}
	}
}

void MeshBuffer::ReverseTriangleOrder(IndexBuffer* ibuffer)
{
	unsigned int const numTriangles = ibuffer->GetNumPrimitives();
	for (unsigned int t = 0; t < numTriangles; ++t)
	{
		unsigned int v0, v1, v2;
		ibuffer->GetTriangle(t, v0, v1, v2);
		ibuffer->SetTriangle(t, v0, v2, v1);
	}
}