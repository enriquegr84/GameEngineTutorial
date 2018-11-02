// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)


#include "Core/Logger/Logger.h"
#include "GL4IndexBuffer.h"

GL4IndexBuffer::GL4IndexBuffer(IndexBuffer const* ibuffer)
    :
    GL4Buffer(ibuffer, GL_ELEMENT_ARRAY_BUFFER)
{
    Initialize();
}

eastl::shared_ptr<GraphicObject> GL4IndexBuffer::Create(void*, GraphicObject const* object)
{
    if (object->GetType() == GE_INDEX_BUFFER)
    {
        return eastl::make_shared<GL4IndexBuffer>(
            static_cast<IndexBuffer const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}

void GL4IndexBuffer::Enable()
{
    glBindBuffer(mType, mGLHandle);
}

void GL4IndexBuffer::Disable()
{
    glBindBuffer(mType, 0);
}
