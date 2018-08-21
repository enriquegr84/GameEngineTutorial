// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "SamplerState.h"

SamplerState::SamplerState()
    :
    mFilter(MIN_P_MAG_P_MIP_P),
    mMipLODBias(0.0f),
    mMaxAnisotropy(1),
    mComparison(NEVER),
    mBorderColor({ 1.0f, 1.0f, 1.0f, 1.0f }),
    mMinLOD(-eastl::numeric_limits<float>::max()),
    mMaxLOD(eastl::numeric_limits<float>::max())
{
    mType = GE_SAMPLER_STATE;

    mMode[0] = CLAMP;
    mMode[1] = CLAMP;
    mMode[2] = CLAMP;
}

