// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11TEXTURE1ARRAY_H
#define DX11TEXTURE1ARRAY_H

#include "Graphic/Resource/Texture/Texture1Array.h"
#include "DX11TextureArray.h"

class GRAPHIC_ITEM DX11Texture1Array : public DX11TextureArray
{
public:
    // Construction.
    DX11Texture1Array(ID3D11Device* device, Texture1Array const* textureArray);
    static eastl::shared_ptr<GraphicObject> Create(void* device, GraphicObject const* object);

    // Member access.
    inline Texture1Array* GetTextureArray() const;
    inline ID3D11Texture1D* GetDXTextureArray() const;

private:
    // Support for construction.
    void CreateStaging(ID3D11Device* device, D3D11_TEXTURE1D_DESC const& tx);
    void CreateSRView(ID3D11Device* device, D3D11_TEXTURE1D_DESC const& tx);
    void CreateUAView(ID3D11Device* device, D3D11_TEXTURE1D_DESC const& tx);
};

inline Texture1Array* DX11Texture1Array::GetTextureArray() const
{
    return static_cast<Texture1Array*>(mGObject);
}

inline ID3D11Texture1D* DX11Texture1Array::GetDXTextureArray() const
{
    return static_cast<ID3D11Texture1D*>(mDXObject);
}

#endif