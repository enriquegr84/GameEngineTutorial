// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef GL4DRAWINGSTATE_H
#define GL4DRAWINGSTATE_H

#include "Graphic/State/DrawingState.h"
#include "Graphic/Renderer/OpenGL4/Resource/GL4GraphicObject.h"

class GRAPHIC_ITEM GL4DrawingState : public GL4GraphicObject
{
protected:
    // Abstract base class.
    GL4DrawingState(DrawingState const* gtState);
};

#endif