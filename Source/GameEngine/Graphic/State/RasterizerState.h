// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef RASTERIZERSTATE_H
#define RASTERIZERSTATE_H

#include "Graphic/State/DrawingState.h"

/*
	Rasterizer customizes shader's behaviors. A variety of raterizer state options
	exist. An important one of them is the culling mode in which models will be drawn or not.
*/
class GRAPHIC_ITEM RasterizerState : public DrawingState
{
public:
    enum GRAPHIC_ITEM FillMode
    {
        FILL_SOLID,
        FILL_WIREFRAME
    };

    enum GRAPHIC_ITEM CullMode
    {
        CULL_NONE,
        CULL_FRONT,
        CULL_BACK
    };

    // Construction.
    RasterizerState();

    // Member access.  The members are intended to be write-once before
    // you create an associated graphics state.
    FillMode fillMode;              // default: FILL_SOLID
    CullMode cullMode;              // default: CULL_BACK
    bool frontCCW;                  // default: true
    int depthBias;                  // default: 0
    float depthBiasClamp;           // default: 0
    float slopeScaledDepthBias;     // default: 0
    bool enableDepthClip;           // default: true
    bool enableScissor;             // default: false
    bool enableMultisample;         // default: false
    bool enableAntialiasedLine;     // default: false
};

#endif