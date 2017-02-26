// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef HLSLVISUALPROGRAM_H
#define HLSLVISUALPROGRAM_H

#include "Graphic/Shader/VisualProgram.h"

/*
	HLSL is a general-purpose language for the GPU side and DirectX is the 
	bridge between the CPU system side and the GPU system side.
	Direct3D is the system for drawing 3D graphics and it defines a sequence
	of steps to present graphics to the screen, known as D3DGraphic Pipeline.
*/
class GRAPHIC_ITEM HLSLVisualProgram : public VisualProgram
{
public:
    // A simple stub to add HLSL as part of the program type. This allows
    // polymorphism for the program factory classes, which in turn allows
    // us to hide the graphics-API-dependent program factory used by the
    // Window class (have a member mProgramFactory similar to mEngine that
    // is created according to the desired graphics API).
    virtual ~HLSLVisualProgram();
    HLSLVisualProgram();
};

#endif