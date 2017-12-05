// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef LIGHTCAMERAGEOMETRY_H
#define LIGHTCAMERAGEOMETRY_H

#include "Graphic/GraphicStd.h"

#include "Mathematic/Algebra/Matrix4x4.h"

struct GRAPHIC_ITEM LightCameraGeometry
{
    // Construction.
    LightCameraGeometry();

    Vector4<float> lightModelPosition;      // default: (0,0,0,1)
    Vector4<float> lightModelDirection;     // default: (0,0,-1,0)
    Vector4<float> lightModelUp;            // default: (0,1,0,0)
    Vector4<float> lightModelRight;         // default: (1,0,0,0)

    Vector4<float> cameraModelPosition;     // default: (0,0,0,1)
    Vector4<float> cameraModelDirection;    // default: (0,0,-1,0)
    Vector4<float> cameraModelUp;           // default: (0,1,0,0)
    Vector4<float> cameraModelRight;        // default: (1,0,0,0)
};

#endif