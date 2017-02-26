// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.3 (2016/08/29)

#include "MeshFactory.h"

MeshFactory::~MeshFactory()
{
}

MeshFactory::MeshFactory()
    :
    mIndexSize(sizeof(unsigned int)),
    mVBUsage(Resource::IMMUTABLE),
    mIBUsage(Resource::IMMUTABLE),
    mOutside(true),
    mPositions(nullptr),
    mNormals(nullptr),
    mTangents(nullptr),
    mBitangents(nullptr)
{
    for (int i = 0; i < VA_MAX_TCOORD_UNITS; ++i)
    {
        mAssignTCoords[i] = false;
        mTCoords[i] = nullptr;
    }
}

eastl::shared_ptr<Visual> MeshFactory::CreateTriangle(unsigned int numSamples,
    float xExtent, float yExtent)
{
    // Quantities derived from inputs.
    float inv = 1.0f / (static_cast<float>(numSamples)-1.0f);
    unsigned int numVertices = numSamples*(numSamples + 1)/2;
    unsigned int numTriangles = (numSamples - 1)*(numSamples - 1);

    // Generate geometry.
    eastl::shared_ptr<VertexBuffer> vbuffer = CreateVBuffer(numVertices);
    if (!vbuffer)
    {
        return nullptr;
    }

    Vector3<float> pos;
    Vector3<float> nor{ 0.0f, 0.0f, 1.0f };
    Vector3<float> tan{ 1.0f, 0.0f, 0.0f };
    Vector3<float> bin{ 0.0f, 1.0f, 0.0f };  // = Cross(nor,tan)
    Vector2<float> tcd;
    pos[2] = 0.0f;
    for (unsigned int i1 = 0, i = 0; i1 < numSamples; ++i1)
    {
        tcd[1] = i1*inv;
        pos[1] = tcd[1] * yExtent;
        for (unsigned int i0 = 0; i0 + i1 < numSamples; ++i0, ++i)
        {
            tcd[0] = i0*inv;
            pos[0] = tcd[0] * xExtent;

            SetPosition(i, pos);
            SetNormal(i, nor);
            SetTangent(i, tan);
            SetBinormal(i, bin);
            SetTCoord(i, tcd);
        }
    }

    // Generate indices.
	eastl::shared_ptr<IndexBuffer> ibuffer = CreateIBuffer(numTriangles);
    if (!ibuffer)
    {
        return nullptr;
    }

    int y0 = 0, y1 = numSamples;
    unsigned int t = 0;
    for (unsigned int i1 = 0; i1 < numSamples - 2; ++i1)
    {
        int bot0 = y0, bot1 = bot0 + 1, top0 = y1, top1 = y1 + 1;
        for (unsigned int i0 = 0; i0 + i1 < numSamples - 2; ++i0)
        {
            ibuffer->SetTriangle(t++, bot0, bot1, top0);
            ibuffer->SetTriangle(t++, bot1, top1, top0);
            bot0 = bot1++;
            top0 = top1++;
        }
        ibuffer->SetTriangle(t++, bot0, bot1, top0);
        y0 = y1;
        y1 = top0 + 1;
    }
    ibuffer->SetTriangle(t++, y0, y0 + 1, y1);

    // Create the mesh.
	eastl::shared_ptr<Visual> visual = eastl::make_shared<Visual>(vbuffer, ibuffer);
    if (visual)
    {
        visual->UpdateModelBound();
    }
    return visual;
}

eastl::shared_ptr<VertexBuffer> MeshFactory::CreateVBuffer(unsigned int numVertices)
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

        // Get texture coordinate channels that are to be assigned values.
        // Clear the mAssignTCoords element in case any elements were set by a
        // previous mesh factory creation call.
		eastl::set<DFType> required;
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

        vbuffer->SetUsage(mVBUsage);
    }
    return vbuffer;
}

eastl::shared_ptr<IndexBuffer> MeshFactory::CreateIBuffer(unsigned int numTriangles)
{
    auto ibuffer = eastl::make_shared<IndexBuffer>(IP_TRIMESH, numTriangles, mIndexSize);
    if (ibuffer)
    {
        ibuffer->SetUsage(mIBUsage);
    }
    return ibuffer;
}

char* MeshFactory::GetGeometricChannel(
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
                    channel + i*mVFormat.GetVertexSize());
                tuple4[3] = w;
            }
        }
    }
    return channel;
}

void MeshFactory::SetPosition(unsigned int i, Vector3<float> const& pos)
{
    Position(i) = pos;
}

void MeshFactory::SetNormal(unsigned int i, Vector3<float> const& nor)
{
    if (mNormals)
    {
        Normal(i) = nor;
    }
}

void MeshFactory::SetTangent(unsigned int i, Vector3<float> const& tan)
{
    if (mTangents)
    {
        Tangent(i) = tan;
    }
}

void MeshFactory::SetBinormal(unsigned int i, Vector3<float> const& bin)
{
    if (mBitangents)
    {
        Bitangent(i) = bin;
    }
}

void MeshFactory::SetTCoord(unsigned int i, Vector2<float> const& tcd)
{
    for (unsigned int unit = 0; unit < VA_MAX_TCOORD_UNITS; ++unit)
    {
        if (mAssignTCoords[unit])
        {
            TCoord(unit, i) = tcd;
        }
    }
}