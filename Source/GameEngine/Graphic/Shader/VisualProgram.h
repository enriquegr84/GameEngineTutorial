// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef VISUALPROGRAM_H
#define VISUALPROGRAM_H

#include "GeometryShader.h"
#include "VertexShader.h"
#include "PixelShader.h"

/*
	Main interface for any graphic device. HLSL uses the class as is. GLSL 
	derives from the class to store the shader and program handles.
*/
class GRAPHIC_ITEM VisualProgram
{
public:
    virtual ~VisualProgram();
    VisualProgram();

    // Member access.
    inline eastl::shared_ptr<VertexShader> const& GetVShader() const;
    inline eastl::shared_ptr<PixelShader> const& GetPShader() const;
    inline eastl::shared_ptr<GeometryShader> const& GetGShader() const;

public:
    // INTERNAL USE ONLY. These are used during factory creation of shaders
    // and programs. The graphics engines need only const-access to the
    // shader objects.
    inline void SetVShader(eastl::shared_ptr<VertexShader> const& shader);
    inline void SetPShader(eastl::shared_ptr<PixelShader> const& shader);
    inline void SetGShader(eastl::shared_ptr<GeometryShader> const& shader);

private:
	eastl::shared_ptr<VertexShader> mVShader;
	eastl::shared_ptr<PixelShader> mPShader;
	eastl::shared_ptr<GeometryShader> mGShader;
};


inline eastl::shared_ptr<VertexShader> const& VisualProgram::GetVShader() const
{
    return mVShader;
}

inline eastl::shared_ptr<PixelShader> const& VisualProgram::GetPShader() const
{
    return mPShader;
}

inline eastl::shared_ptr<GeometryShader> const& VisualProgram::GetGShader()
    const
{
    return mGShader;
}

inline void VisualProgram::SetVShader(eastl::shared_ptr<VertexShader> const& shader)
{
    mVShader = shader;
}

inline void VisualProgram::SetPShader(eastl::shared_ptr<PixelShader> const& shader)
{
    mPShader = shader;
}

inline void VisualProgram::SetGShader(eastl::shared_ptr<GeometryShader> const& shader)
{
    mGShader = shader;
}

#endif
