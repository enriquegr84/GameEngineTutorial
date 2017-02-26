// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11TEXTURESINGLE_H
#define DX11TEXTURESINGLE_H

#include "Graphic/Resource/Texture/TextureSingle.h"
#include "DX11Texture.h"

class GRAPHIC_ITEM DX11TextureSingle : public DX11Texture
{
protected:
    // Abstract base class, a shim to distinguish between single textures and
    // texture arrays.
    DX11TextureSingle(TextureSingle const* gtTextureSingle);

public:
    // Member access.
    inline TextureSingle* GetTextureSingle() const;
};

inline TextureSingle* DX11TextureSingle::GetTextureSingle() const
{
    return static_cast<TextureSingle*>(mGObject);
}

#endif