// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef GL4INDEXBUFFER_H
#define GL4INDEXBUFFER_H

#include "Graphic/Resource/Buffer/IndexBuffer.h"
#include "GL4Buffer.h"

class GRAPHIC_ITEM GL4IndexBuffer : public GL4Buffer
{
public:
    // Construction.
    GL4IndexBuffer(IndexBuffer const* ibuffer);
    static eastl::shared_ptr<GraphicObject> Create(void* unused, GraphicObject const* object);

    // Member access.
    inline IndexBuffer* GetIndexBuffer() const;

    // Support for drawing geometric primitives.
    void Enable();
    void Disable();
};

inline IndexBuffer* GL4IndexBuffer::GetIndexBuffer() const
{
    return static_cast<IndexBuffer*>(mGObject);
}

#endif