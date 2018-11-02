// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)


#include "Core/Logger/Logger.h"

#include "GL4TextureRT.h"

GL4TextureRT::GL4TextureRT(TextureRT const* texture)
    :
    GL4Texture2(texture)
{
}

eastl::shared_ptr<GraphicObject> GL4TextureRT::Create(void*, GraphicObject const* object)
{
    if (object->GetType() == GE_TEXTURE_RT)
    {
        return eastl::make_shared<GL4TextureRT>(
            static_cast<TextureRT const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}

bool GL4TextureRT::CanAutoGenerateMipmaps() const
{
    auto texture = GetTexture();

    return texture && texture->HasMipmaps() && texture->IsAutogenerateMipmaps();
}
