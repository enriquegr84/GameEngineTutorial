// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef SAMPLERSTATE_H
#define SAMPLERSTATE_H

#include "Mathematic/Algebra/Vector4.h"
#include "Graphic/State/DrawingState.h"

class GRAPHIC_ITEM SamplerState : public DrawingState
{
public:
    // The encoding involves minification (MIN), magnification (MAG), and
    // mip-level filtering (MIP).  After each is P (POINT) or L (LINEAR).
    enum GRAPHIC_ITEM Filter
    {
        MIN_P_MAG_P_MIP_P,
        MIN_P_MAG_P_MIP_L,
        MIN_P_MAG_L_MIP_P,
        MIN_P_MAG_L_MIP_L,
        MIN_L_MAG_P_MIP_P,
        MIN_L_MAG_P_MIP_L,
        MIN_L_MAG_L_MIP_P,
        MIN_L_MAG_L_MIP_L,
        ANISOTROPIC,
        COMPARISON_MIN_P_MAG_P_MIP_P,
        COMPARISON_MIN_P_MAG_P_MIP_L,
        COMPARISON_MIN_P_MAG_L_MIP_P,
        COMPARISON_MIN_P_MAG_L_MIP_L,
        COMPARISON_MIN_L_MAG_P_MIP_P,
        COMPARISON_MIN_L_MAG_P_MIP_L,
        COMPARISON_MIN_L_MAG_L_MIP_P,
        COMPARISON_MIN_L_MAG_L_MIP_L,
        COMPARISON_ANISOTROPIC
    };

    // Modes for handling texture coordinates at texture-image boundaries.
    enum GRAPHIC_ITEM Mode
    {
        WRAP,
        MIRROR,
        CLAMP,
        BORDER,
        MIRROR_ONCE
    };

    enum GRAPHIC_ITEM Comparison
    {
        NEVER,
        LESS,
        EQUAL,
        LESS_EQUAL,
        GREATER,
        NOT_EQUAL,
        GREATER_EQUAL,
        ALWAYS
    };

    // Construction.
    SamplerState();

    // Member access.  The members are intended to be write-once before
    // you create an associated graphics state.
    Filter mFilter;
    Mode mMode[3];
    float mMipLODBias;
    unsigned int mMaxAnisotropy;
    Comparison mComparison;
    Vector4<float> mBorderColor;
    float mMinLOD;
    float mMaxLOD;

public:
    // For use by the Shader class for storing reflection information.
    static int const mShaderDataLookup = 6;
};

#endif
