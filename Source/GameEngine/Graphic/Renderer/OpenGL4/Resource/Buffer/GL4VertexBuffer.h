// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef GL4VERTEXBUFFER_H
#define GL4VERTEXBUFFER_H

#include "Graphic/Resource/Buffer/VertexBuffer.h"
#include "GL4Buffer.h"

class GRAPHIC_ITEM GL4VertexBuffer : public GL4Buffer
{
public:
    // Construction.
    GL4VertexBuffer(VertexBuffer const* vbuffer);
    static eastl::shared_ptr<GraphicObject> Create(void* unused, GraphicObject const* object);

    // Member access.
    inline VertexBuffer* GetVertexBuffer() const;

    // TODO: Drawing support?  Currently, the enable/disable is in the
    // GL4InputLayout class, which assumes OpenGL 4.3 or later.  What if the
    // application machine does not have OpenGL 4.3?  Fall back to the
    // glBindBuffer paradigm?
};

inline VertexBuffer* GL4VertexBuffer::GetVertexBuffer() const
{
    return static_cast<VertexBuffer*>(mGObject);
}

#endif
