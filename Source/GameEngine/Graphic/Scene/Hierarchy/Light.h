// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef LIGHT_H
#define LIGHT_H

#include "GameEngineStd.h"

#include "Graphic/Effect/Lighting.h"

#include "ViewVolume.h"


class GRAPHIC_ITEM Light : public ViewVolume
{
public:
    // Construction. The depth range for DirectX is [0,1] and for OpenGL is
    // [-1,1]. For DirectX, set isDepthRangeZeroToOne to true. For OpenGL,
    // set isDepthRangeZeroOne to false.
    Light(bool isPerspective, bool isDepthRangeZeroOne);

    eastl::shared_ptr<Lighting> lighting;
};

#endif
