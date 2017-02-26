// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11TEXTURERS_H
#define DX11TEXTURERS_H

#include "Graphic/Resource/Texture/TextureRT.h"
#include "DX11Texture2.h"

class GRAPHIC_ITEM DX11TextureRT : public DX11Texture2
{
public:
    // Construction and destruction.
    virtual ~DX11TextureRT();
    DX11TextureRT(ID3D11Device* device, TextureRT const* texture);
    DX11TextureRT(ID3D11Device* device, DX11TextureRT const* dxSharedTexture);
    static eastl::shared_ptr<GraphicObject> Create(void* device, GraphicObject const* object);

    // Member access.
    inline ID3D11RenderTargetView* GetRTView() const;

private:
    // Support for construction.
    void CreateRTView(ID3D11Device* device, D3D11_TEXTURE2D_DESC const& tx);

    ID3D11RenderTargetView* mRTView;
};

inline ID3D11RenderTargetView* DX11TextureRT::GetRTView() const
{
    return mRTView;
}

#endif