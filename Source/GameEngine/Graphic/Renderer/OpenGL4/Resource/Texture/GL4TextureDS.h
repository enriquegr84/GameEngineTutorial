// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef GL4TEXTUREDS_H
#define GL4TEXTUREDS_H

#include "Graphic/Resource/Texture/TextureDS.h"
#include "GL4Texture2.h"

class GRAPHIC_ITEM GL4TextureDS : public GL4Texture2
{
public:
    // Construction.
    GL4TextureDS(TextureDS const* texture);
    static eastl::shared_ptr<GraphicObject> Create(void* unused, GraphicObject const* object);

    // Member access.
    inline TextureDS* GetTexture() const;

    // Returns true of mipmaps need to be generated.
    virtual bool CanAutoGenerateMipmaps() const override;

private:
};

inline TextureDS* GL4TextureDS::GetTexture() const
{
    return static_cast<TextureDS*>(mGObject);
}

#endif