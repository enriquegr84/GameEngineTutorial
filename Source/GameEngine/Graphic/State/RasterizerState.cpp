// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "RasterizerState.h"

RasterizerState::RasterizerState()
    :
    fillMode(FILL_SOLID),
    cullMode(CULL_BACK),
    frontCCW(true),
    depthBias(0),
    depthBiasClamp(0.0f),
    slopeScaledDepthBias(0.0f),
    enableDepthClip(true),
    enableScissor(false),
    enableMultisample(false),
    enableAntialiasedLine(false)
{
    mType = GE_RASTERIZER_STATE;
}

