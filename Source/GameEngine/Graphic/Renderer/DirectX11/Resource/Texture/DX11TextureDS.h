// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/11/13)

#ifndef DX11TEXTUREDS_H
#define DX11TEXTUREDS_H

#include "Graphic/Resource/Texture/TextureDS.h"
#include "DX11Texture2.h"

class GRAPHIC_ITEM DX11TextureDS : public DX11Texture2
{
public:
    // Construction and destruction.
    virtual ~DX11TextureDS();
    DX11TextureDS(ID3D11Device* device, TextureDS const* texture);
    DX11TextureDS(ID3D11Device* device, DX11TextureDS const* dxSharedTexture);
    static eastl::shared_ptr<GraphicObject> Create(void* device, GraphicObject const* object);

    // Member access.
    inline ID3D11DepthStencilView* GetDSView() const;

private:
    // Support for construction.
    void CreateDSView(ID3D11Device* device);
    void CreateDSSRView(ID3D11Device* device);
    DXGI_FORMAT GetDepthResourceFormat(DXGI_FORMAT depthFormat);
    DXGI_FORMAT GetDepthSRVFormat(DXGI_FORMAT depthFormat);

    ID3D11DepthStencilView* mDSView;
};

inline ID3D11DepthStencilView* DX11TextureDS::GetDSView() const
{
    return mDSView;
}

#endif