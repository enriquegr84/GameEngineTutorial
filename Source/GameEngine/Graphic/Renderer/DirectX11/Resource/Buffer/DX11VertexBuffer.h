// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11VERTEXBUFFER_H
#define DX11VERTEXBUFFER_H

#include "Graphic/Resource/Buffer/VertexBuffer.h"
#include "DX11Buffer.h"

class GRAPHIC_ITEM DX11VertexBuffer : public DX11Buffer
{
public:
    // Construction.
    DX11VertexBuffer(ID3D11Device* device, VertexBuffer const* vbuffer);
    static eastl::shared_ptr<GraphicObject> Create(void* device, GraphicObject const* object);

    // Member access.
    inline VertexBuffer* GetVertexBuffer() const;

    // Drawing support.
    void Enable(ID3D11DeviceContext* context);
    void Disable(ID3D11DeviceContext* context);
};

inline VertexBuffer* DX11VertexBuffer::GetVertexBuffer() const
{
    return static_cast<VertexBuffer*>(mGObject);
}

#endif