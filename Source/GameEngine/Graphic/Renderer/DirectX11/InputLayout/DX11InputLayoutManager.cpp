// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "DX11InputLayoutManager.h"

DX11InputLayoutManager::~DX11InputLayoutManager()
{
    if (mMap.HasElements())
    {
        LogWarning("Input layout map is not empty on destruction.");
        UnbindAll();
    }
}

DX11InputLayoutManager::DX11InputLayoutManager()
{
}

DX11InputLayout* DX11InputLayoutManager::Bind(ID3D11Device* device,
    VertexBuffer const* vbuffer, Shader const* vshader)
{
    eastl::shared_ptr<DX11InputLayout> layout;
    if (vshader)
    {
        if (vbuffer)
        {
            if (!mMap.Get(eastl::make_pair(vbuffer, vshader), layout))
            {
                layout = eastl::make_shared<DX11InputLayout>(device, vbuffer, vshader);
                mMap.Insert(eastl::make_pair(vbuffer, vshader), layout);
            }
        }
        // else: A null vertex buffer is passed when an effect wants to
        // bypass the input assembler.
    }
    else
    {
        LogError("Vertex shader must be nonnull.");
    }
    return layout.get();
}

bool DX11InputLayoutManager::Unbind(VertexBuffer const* vbuffer)
{
    if (vbuffer)
    {
		eastl::vector<VBSPair> matches;
        mMap.GatherMatch(vbuffer, matches);
        for (auto match : matches)
        {
			eastl::shared_ptr<DX11InputLayout> layout;
            mMap.Remove(match, layout);
        }
        return true;
    }
    else
    {
        LogError("Vertex buffer must be nonnull.");
        return false;
    }
}

bool DX11InputLayoutManager::Unbind(Shader const* vshader)
{
    if (vshader)
    {
		eastl::vector<VBSPair> matches;
        mMap.GatherMatch(vshader, matches);
        for (auto match : matches)
        {
			eastl::shared_ptr<DX11InputLayout> layout;
            mMap.Remove(match, layout);
        }
        return true;
    }
    else
    {
        LogError("Vertex shader must be nonnull.");
        return false;
    }
}

void DX11InputLayoutManager::UnbindAll()
{
    mMap.RemoveAll();
}

bool DX11InputLayoutManager::HasElements() const
{
    return mMap.HasElements();
}

DX11InputLayoutManager::LayoutMap::~LayoutMap()
{
}

DX11InputLayoutManager::LayoutMap::LayoutMap()
{
}

void DX11InputLayoutManager::LayoutMap::GatherMatch(
    VertexBuffer const* vbuffer, eastl::vector<VBSPair>& matches)
{
    mMutex.lock();
    {
        for (auto vbs : mMap)
        {
            if (vbuffer == vbs.first.first)
            {
                matches.push_back(vbs.first);
            }
        }
    }
    mMutex.unlock();
}

void DX11InputLayoutManager::LayoutMap::GatherMatch(
	Shader const* vshader, eastl::vector<VBSPair>& matches)
{
    mMutex.lock();
    {
        for (auto vbs : mMap)
        {
            if (vshader == vbs.first.second)
            {
                matches.push_back(vbs.first);
            }
        }
    }
    mMutex.unlock();
}
