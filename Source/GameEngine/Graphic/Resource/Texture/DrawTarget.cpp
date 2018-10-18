// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Core/Logger/Logger.h"
#include "DrawTarget.h"

DrawTarget::~DrawTarget()
{
    msLFDMutex.lock();
    {
        for (auto listener : msLFDSet)
        {
            listener->OnDestroy(this);
        }
    }
    msLFDMutex.unlock();
}

DrawTarget::DrawTarget(unsigned int numRenderTargets, DFType rtFormat,
    unsigned int width, unsigned int height, bool hasRTMipmaps,
    bool createRTStorage, DFType dsFormat, bool createDSStorage)
{
    if (numRenderTargets > 0)
    {
        mRTTextures.resize(numRenderTargets);
        for (auto& texture : mRTTextures)
        {
            texture = eastl::make_shared<TextureRT>(rtFormat, width, height,
                hasRTMipmaps, createRTStorage);
        }

        if (dsFormat != DF_UNKNOWN)
        {
            if (DataFormat::IsDepth(dsFormat))
            {
                mDSTexture = eastl::make_shared<TextureDS>(dsFormat, width,
                    height, createDSStorage);
            }
            else
            {
                LogError("Invalid depth-stencil format.");
            }
        }
    }
    else
    {
        LogError("Number of targets must be at least one.");
    }
}

unsigned int DrawTarget::GetNumTargets() const
{
    return static_cast<unsigned int>(mRTTextures.size());
}

DFType DrawTarget::GetRTFormat() const
{
    if (mRTTextures.size() > 0)
    {
        return mRTTextures[0]->GetFormat();
    }

    LogError("Unexpected condition.");
    return DF_UNKNOWN;
}

unsigned int DrawTarget::GetWidth() const
{
    if (mRTTextures.size() > 0)
    {
        return mRTTextures[0]->GetWidth();
    }

    LogError("Unexpected condition.");
    return 0;
}

unsigned int DrawTarget::GetHeight() const
{
    if (mRTTextures.size() > 0)
    {
        return mRTTextures[0]->GetHeight();
    }

    LogError("Unexpected condition.");
    return 0;
}

bool DrawTarget::HasRTMipmaps() const
{
    if (mRTTextures.size() > 0)
    {
        return mRTTextures[0]->HasMipmaps();
    }

    LogError("Unexpected condition.");
    return false;
}

DFType DrawTarget::GetDSFormat() const
{
    if (mDSTexture)
    {
        return mDSTexture->GetFormat();
    }

    LogError("Unexpected condition.");
    return DF_UNKNOWN;
}

eastl::shared_ptr<TextureRT> const DrawTarget::GetRTTexture(unsigned int i) const
{
    if (i < static_cast<int>(mRTTextures.size()))
    {
        return mRTTextures[i];
    }

    LogError("Invalid index.");
    return nullptr;
}

eastl::shared_ptr<TextureDS> const DrawTarget::GetDSTexture() const
{
    return mDSTexture;
}

void DrawTarget::AutogenerateRTMipmaps()
{
    if (HasRTMipmaps())
    {
        for (auto& texture : mRTTextures)
        {
            texture->AutogenerateMipmaps();
        }
    }
}

bool DrawTarget::IsAutogenerateRTMipmaps() const
{
    if (mRTTextures.size() > 0)
    {
        return mRTTextures[0]->IsAutogenerateMipmaps();
    }

    LogError("Unexpected condition.");
    return false;
}

void DrawTarget::SubscribeForDestruction(eastl::shared_ptr<ListenerForDestruction> const& listener)
{
    msLFDMutex.lock();
    {
        msLFDSet.insert(listener);
    }
    msLFDMutex.unlock();
}

void DrawTarget::UnsubscribeForDestruction(eastl::shared_ptr<ListenerForDestruction> const& listener)
{
    msLFDMutex.lock();
    {
        msLFDSet.erase(listener);
    }
    msLFDMutex.unlock();
}


std::recursive_mutex DrawTarget::msLFDMutex;
eastl::set<eastl::shared_ptr<DrawTarget::ListenerForDestruction>> DrawTarget::msLFDSet;
