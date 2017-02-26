// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11CONSTANTBUFFER_H
#define DX11CONSTANTBUFFER_H

#include "Graphic/Resource/Buffer/ConstantBuffer.h"
#include "DX11Buffer.h"

class GRAPHIC_ITEM DX11ConstantBuffer : public DX11Buffer
{
public:
    // Construction.
    DX11ConstantBuffer(ID3D11Device* device, ConstantBuffer const* cbuffer);
    static eastl::shared_ptr<GraphicObject> Create(void* device, GraphicObject const* object);

    // Member access.
    inline ConstantBuffer* GetConstantBuffer() const;
};

inline ConstantBuffer* DX11ConstantBuffer::GetConstantBuffer() const
{
    return static_cast<ConstantBuffer*>(mGObject);
}

#endif
