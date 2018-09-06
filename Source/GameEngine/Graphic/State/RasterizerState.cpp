// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "RasterizerState.h"

RasterizerState::RasterizerState()
    :
    mFillMode(FILL_SOLID),
    mCullMode(CULL_NONE),
    mFrontCCW(true),
    mDepthBias(0),
    mDepthBiasClamp(0.0f),
    mSlopeScaledDepthBias(0.0f),
    mEnableDepthClip(true),
    mEnableScissor(false),
    mEnableMultisample(false),
    mEnableAntialiasedLine(false)
{
    mType = GE_RASTERIZER_STATE;
}

