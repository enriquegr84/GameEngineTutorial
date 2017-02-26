// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef COMPUTEPROGRAM_H
#define COMPUTEPROGRAM_H

#include "ComputeShader.h"

class GRAPHIC_ITEM ComputeProgram
{
public:
    // HLSL uses the class as is.  GLSL derives from the class to store
    // the shader and program handles.
    virtual ~ComputeProgram();
    ComputeProgram();

    // Member access.
    inline eastl::shared_ptr<ComputeShader> const& GetCShader() const;

public:
    // INTERNAL USE ONLY.  This is used during factory creation of shaders
    // and programs.  The graphics engines need only const-access to the
    // shader objects.
    inline void SetCShader(eastl::shared_ptr<ComputeShader> const& shader);

private:
	eastl::shared_ptr<ComputeShader> mCShader;
};


inline eastl::shared_ptr<ComputeShader> const& ComputeProgram::GetCShader() const
{
    return mCShader;
}

inline void ComputeProgram::SetCShader(eastl::shared_ptr<ComputeShader> const& shader)
{
    mCShader = shader;
}

#endif
