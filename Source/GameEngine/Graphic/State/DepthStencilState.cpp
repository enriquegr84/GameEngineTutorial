// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "DepthStencilState.h"

DepthStencilState::DepthStencilState()
    :
    mDepthEnable(true),
    mWriteMask(MASK_ALL),
    mComparison(LESS_EQUAL),
    mStencilEnable(false),
    mStencilReadMask(0xFF),
    mStencilWriteMask(0xFF),
    mReference(0)
{
    mType = GE_DEPTH_STENCIL_STATE;

    mFrontFace.fail = OP_KEEP;
    mFrontFace.depthFail = OP_KEEP;
    mFrontFace.pass = OP_KEEP;
    mFrontFace.comparison = ALWAYS;
    mBackFace.fail = OP_KEEP;
    mBackFace.depthFail = OP_KEEP;
    mBackFace.pass = OP_KEEP;
    mBackFace.comparison = ALWAYS;
}

