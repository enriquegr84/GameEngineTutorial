// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Mathematic/Function/Constants.h"
#include "Lighting.h"

Lighting::Lighting()
    :
    mAmbient({ 1.0f, 1.0f, 1.0f, 1.0f }),
    mDiffuse({ 1.0f, 1.0f, 1.0f, 1.0f }),
    mSpecular({ 1.0f, 1.0f, 1.0f, 1.0f }),
    mSpotCutoff({ (float)GE_C_HALF_PI, 0.0f, 1.0f, 1.0f }),
    mAttenuation({ 1.0f, 0.0f, 0.0f, 1.0f }),
	mPosition{ 0.f, 0.f, 0.f }, mDirection{ 0.f, 0.f, 1.f },
	mType(LT_POINT)
{
}
