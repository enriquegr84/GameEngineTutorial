// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef GL4CONSTANTBUFFER_H
#define GL4CONSTANTBUFFER_H

#include "Graphic/Resource/Buffer/ConstantBuffer.h"
#include "GL4Buffer.h"

class GRAPHIC_ITEM GL4ConstantBuffer : public GL4Buffer
{
public:
    // Construction.
    GL4ConstantBuffer(ConstantBuffer const* cbuffer);
    static eastl::shared_ptr<GraphicObject> Create(void* unused, GraphicObject const* object);

    // Member access.
    inline ConstantBuffer* GetConstantBuffer() const;

    // Bind the constant buffer data to the specified uniform buffer unit.
    void AttachToUnit(GLint uniformBufferUnit);
};

inline ConstantBuffer* GL4ConstantBuffer::GetConstantBuffer() const
{
    return static_cast<ConstantBuffer*>(mGObject);
}

#endif