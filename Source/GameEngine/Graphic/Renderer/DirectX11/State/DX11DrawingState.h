// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef DX11DRAWINGSTATE_H
#define DX11DRAWINGSTATE_H

#include "Graphic/State/DrawingState.h"
#include "Graphic/Renderer/DirectX11/Resource/DX11GraphicObject.h"

class GRAPHIC_ITEM DX11DrawingState : public DX11GraphicObject
{
protected:
    // Abstract base class.
    DX11DrawingState(DrawingState const* gState);
};

#endif