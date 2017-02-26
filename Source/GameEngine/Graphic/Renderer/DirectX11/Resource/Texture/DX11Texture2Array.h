// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11TEXTURE2ARRAY_H
#define DX11TEXTURE2ARRAY_H

#include "Graphic/Resource/Texture/Texture2Array.h"
#include "DX11TextureArray.h"

class GRAPHIC_ITEM DX11Texture2Array : public DX11TextureArray
{
public:
    // Construction.
    DX11Texture2Array(ID3D11Device* device, Texture2Array const* textureArray);
    static eastl::shared_ptr<GraphicObject> Create(void* device, GraphicObject const* object);

    // Member access.
    inline Texture2Array* GetTextureArray() const;
    inline ID3D11Texture2D* GetDXTextureArray() const;

private:
    // Support for construction.
    void CreateStaging(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
    void CreateSRView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
    void CreateUAView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);
};

inline Texture2Array* DX11Texture2Array::GetTextureArray() const
{
    return static_cast<Texture2Array*>(mGObject);
}

inline ID3D11Texture2D* DX11Texture2Array::GetDXTextureArray() const
{
    return static_cast<ID3D11Texture2D*>(mDXObject);
}

#endif