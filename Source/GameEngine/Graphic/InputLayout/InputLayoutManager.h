// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef INPUTLAYOUTMANAGER_H
#define INPUTLAYOUTMANAGER_H

#include "Graphic/GraphicStd.h"

class Shader;
class VertexBuffer;

class GRAPHIC_ITEM InputLayoutManager
{
public:
    // Abstract base interface.
    virtual ~InputLayoutManager() {}
    InputLayoutManager() {}

    virtual bool Unbind(VertexBuffer const* vbuffer) = 0;
    virtual bool Unbind(Shader const* vshader) = 0;
    virtual void UnbindAll() = 0;
    virtual bool HasElements() const = 0;
};

#endif