// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)


#include "Core/Logger/Logger.h"
#include "GL4TextureDS.h"


GL4TextureDS::GL4TextureDS(TextureDS const* texture)
    :
    GL4Texture2(texture)
{
}

eastl::shared_ptr<GraphicObject> GL4TextureDS::Create(void*, GraphicObject const* object)
{
    if (object->GetType() == GE_TEXTURE_DS)
    {
        return eastl::make_shared<GL4TextureDS>(
            static_cast<TextureDS const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}

bool GL4TextureDS::CanAutoGenerateMipmaps() const
{
    return false;
}
