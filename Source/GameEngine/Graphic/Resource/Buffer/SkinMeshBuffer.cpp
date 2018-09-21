// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SkinMeshBuffer.h"

SkinMeshBuffer::SkinMeshBuffer()
	: Buffer(0, 0), mVertexType(VT_STANDARD)
{
	mMaterial = eastl::make_shared<Material>();
}

SkinMeshBuffer::SkinMeshBuffer(VertexFormat const& vformat, uint32_t numVertices,
	uint32_t numPrimitives, size_t indexSize)
	: Buffer(numPrimitives, 0), mVertexType(VT_STANDARD)
{
	mVFormat = vformat;

	mIndice = CreateIBuffer(numPrimitives, indexSize);
	if (!mIndice)
		LogError("Error creating index buffer");
	CreateVBuffer(numVertices);
	if (!mVertice)
		LogError("Error creating vertex buffer");

	mMaterial = eastl::make_shared<Material>();
}

SkinMeshBuffer::~SkinMeshBuffer()
{

}

void SkinMeshBuffer::CreateVBuffer(unsigned int numVertices)
{
	auto vbuffer = eastl::make_shared<VertexBuffer>(mVFormat, numVertices);
	if (vbuffer)
	{
		//Vertex
		{
			// Get the position channel.
			mVertexPositions = GetGeometricChannel(vbuffer, VA_POSITION, 1.0f);
			if (!mVertexPositions)
			{
				LogError("Positions are required.");
				return;
			}

			// Get the optional geometric channels.
			mVertexNormals = GetGeometricChannel(vbuffer, VA_NORMAL, 0.0f);
			mVertexTangents = GetGeometricChannel(vbuffer, VA_TANGENT, 0.0f);
			mVertexBitangents = GetGeometricChannel(vbuffer, VA_BINORMAL, 0.0f);

			// Get color channels that are to be assigned values.
			// Clear the mAssignColors element in case any elements were set by a
			// previous creation call.
			eastl::set<DFType> required;
			required.insert(DF_R32G32B32A32_FLOAT);
			for (unsigned int unit = 0; unit < VA_MAX_COLOR_UNITS; ++unit)
			{
				mVertexColors[unit] = vbuffer->GetChannel(VA_COLOR, unit, required);
				if (mVertexColors[unit])
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
				mVertexTCoords[unit] = vbuffer->GetChannel(VA_TEXCOORD, unit, required);
				if (mVertexTCoords[unit])
				{
					mAssignTCoords[unit] = true;
				}
				else
				{
					mAssignTCoords[unit] = false;
				}
			}
		}

		vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
		mVertice = vbuffer;
	}

	vbuffer = eastl::make_shared<VertexBuffer>(mVFormat, numVertices);
	if (vbuffer)
	{
		//Vertex tangent
		{
			// Get the position channel.
			mVertexTanPositions = GetGeometricChannel(vbuffer, VA_POSITION, 1.0f);
			mVertexTanNormals = GetGeometricChannel(vbuffer, VA_NORMAL, 0.0f);
			mVertexTanTangents = GetGeometricChannel(vbuffer, VA_TANGENT, 0.0f);
			mVertexTanBitangents = GetGeometricChannel(vbuffer, VA_BINORMAL, 0.0f);

			// Get color channels that are to be assigned values.
			// Clear the mAssignColors element in case any elements were set by a
			// previous creation call.
			eastl::set<DFType> required;
			required.insert(DF_R32G32B32A32_FLOAT);
			for (unsigned int unit = 0; unit < VA_MAX_COLOR_UNITS; ++unit)
				mVertexTanColors[unit] = vbuffer->GetChannel(VA_COLOR, unit, required);

			// Get texture coordinate channels that are to be assigned values.
			// Clear the mAssignTCoords element in case any elements were set by a
			// previous creation call.
			required.clear();
			required.insert(DF_R32G32_FLOAT);
			for (unsigned int unit = 0; unit < VA_MAX_TCOORD_UNITS; ++unit)
				mVertexTanTCoords[unit] = vbuffer->GetChannel(VA_TEXCOORD, unit, required);
		}

		vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
		mVerticeTangent = vbuffer;
	}

	vbuffer = eastl::make_shared<VertexBuffer>(mVFormat, numVertices);
	if (vbuffer)
	{
		//Vertex texture coordinates
		{
			// Get the position channel.
			mVertexTCPositions = GetGeometricChannel(vbuffer, VA_POSITION, 1.0f);
			mVertexTCNormals = GetGeometricChannel(vbuffer, VA_NORMAL, 0.0f);
			mVertexTCTangents = GetGeometricChannel(vbuffer, VA_TANGENT, 0.0f);
			mVertexTCBitangents = GetGeometricChannel(vbuffer, VA_BINORMAL, 0.0f);

			// Get color channels that are to be assigned values.
			// Clear the mAssignColors element in case any elements were set by a
			// previous creation call.
			eastl::set<DFType> required;
			required.insert(DF_R32G32B32A32_FLOAT);
			for (unsigned int unit = 0; unit < VA_MAX_COLOR_UNITS; ++unit)
				mVertexTCColors[unit] = vbuffer->GetChannel(VA_COLOR, unit, required);

			// Get texture coordinate channels that are to be assigned values.
			// Clear the mAssignTCoords element in case any elements were set by a
			// previous creation call.
			required.clear();
			required.insert(DF_R32G32_FLOAT);
			for (unsigned int unit = 0; unit < VA_MAX_TCOORD_UNITS; ++unit)
				mVertexTCTCoords[unit] = vbuffer->GetChannel(VA_TEXCOORD, unit, required);

		}

		vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
		mVerticeTCoords = vbuffer;
	}
}

eastl::shared_ptr<IndexBuffer> SkinMeshBuffer::CreateIBuffer(unsigned int numTriangles, size_t indexSize)
{
	auto ibuffer = eastl::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, indexSize);
	if (ibuffer)
	{
		ibuffer->SetUsage(Resource::IMMUTABLE);
	}
	return ibuffer;
}

char* SkinMeshBuffer::GetGeometricChannel(
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

void SkinMeshBuffer::SetPosition(unsigned int i, Vector3<float> const& pos)
{
	Position(i) = pos;
}

void SkinMeshBuffer::SetNormal(unsigned int i, Vector3<float> const& nor)
{
	if (mVertexNormals)
	{
		Normal(i) = nor;
	}
}

void SkinMeshBuffer::SetTangent(unsigned int i, Vector3<float> const& tan)
{
	if (mVertexTangents)
	{
		Tangent(i) = tan;
	}
}

void SkinMeshBuffer::SetBinormal(unsigned int i, Vector3<float> const& bin)
{
	if (mVertexBitangents)
	{
		Bitangent(i) = bin;
	}
}

void SkinMeshBuffer::SetColor(unsigned int i, Vector4<float> const& cl)
{
	for (unsigned int unit = 0; unit < VA_MAX_COLOR_UNITS; ++unit)
	{
		if (mAssignColors[unit])
		{
			Color(unit, i) = cl;
		}
	}
}

void SkinMeshBuffer::SetTCoord(unsigned int i, Vector2<float> const& tcd)
{
	for (unsigned int unit = 0; unit < VA_MAX_TCOORD_UNITS; ++unit)
	{
		if (mAssignTCoords[unit])
		{
			TCoord(unit, i) = tcd;
		}
	}
}

void SkinMeshBuffer::ReverseTriangleOrder(IndexBuffer* ibuffer)
{
	unsigned int const numTriangles = ibuffer->GetNumPrimitives();
	for (unsigned int t = 0; t < numTriangles; ++t)
	{
		unsigned int v0, v1, v2;
		ibuffer->GetTriangle(t, v0, v1, v2);
		ibuffer->SetTriangle(t, v0, v2, v1);
	}
}