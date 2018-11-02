// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef GL4RASETERIZERSTATE_H
#define GL4RASETERIZERSTATE_H

#include "Graphic/State/RasterizerState.h"
#include "GL4DrawingState.h"

class GRAPHIC_ITEM GL4RasterizerState : public GL4DrawingState
{
public:
    // Construction.
    GL4RasterizerState(RasterizerState const* rasterizerState);
    static eastl::shared_ptr<GraphicObject> Create(void* unused, GraphicObject const* object);

    // Member access.
    inline RasterizerState* GetRasterizerState();

    // Enable the rasterizer state.
    void Enable();

private:
    GLenum mFillMode;
    GLenum mCullFace;
    GLenum mFrontFace;
    float mDepthScale;
    float mDepthBias;
    GLboolean mEnableScissor;

    // TODO: D3D11_RASTERIZER_DESC has the following.  We need to determine
    // how to handle these in OpenGL.
    //   DepthBiasClamp
    //   DepthClipEnable
    //   MultisampleEnable
    //   AntialiasedLineEnable

    // Conversions from Engine values to GL4 values.
    static GLenum const msFillMode[];
    static GLenum const msCullFace[];
};

inline RasterizerState* GL4RasterizerState::GetRasterizerState()
{
    return static_cast<RasterizerState*>(mGObject);
}

#endif