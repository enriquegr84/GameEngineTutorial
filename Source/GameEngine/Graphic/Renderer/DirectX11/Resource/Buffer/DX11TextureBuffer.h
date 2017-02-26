// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11TEXTUREBUFFER_H
#define DX11TEXTUREBUFFER_H

#include "Graphic/Resource/Buffer/TextureBuffer.h"
#include "DX11Buffer.h"

class GRAPHIC_ITEM DX11TextureBuffer : public DX11Buffer
{
public:
    // Construction and destruction.
    virtual ~DX11TextureBuffer();
    DX11TextureBuffer(ID3D11Device* device, TextureBuffer const* tbuffer);
    static eastl::shared_ptr<GraphicObject> Create(void* device, GraphicObject const* object);

    // Member access.
    inline TextureBuffer* GetTextureBuffer() const;
    inline ID3D11ShaderResourceView* GetSRView() const;

private:
    // Support for construction.
    void CreateSRView(ID3D11Device* device);

    ID3D11ShaderResourceView* mSRView;
};

inline TextureBuffer* DX11TextureBuffer::GetTextureBuffer() const
{
    return static_cast<TextureBuffer*>(mGObject);
}

inline ID3D11ShaderResourceView* DX11TextureBuffer::GetSRView() const
{
    return mSRView;
}

#endif