// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef GL4DEPTHSTENCILSTATE_H
#define GL4DEPTHSTENCILSTATE_H

#include "Graphic/State/DepthStencilState.h"
#include "GL4DrawingState.h"

class GRAPHIC_ITEM GL4DepthStencilState : public GL4DrawingState
{
public:
    // Construction.
    GL4DepthStencilState(DepthStencilState const* depthStencilState);
    static eastl::shared_ptr<GraphicObject> Create(void* unused, GraphicObject const* object);

    // Member access.
    inline DepthStencilState* GetDepthStencilState();

    // Enable the depth-stencil state.
    void Enable();

private:
    struct Face
    {
        GLenum onFail;
        GLenum onZFail;
        GLenum onZPass;
        GLenum comparison;
    };

    GLboolean mDepthEnable;
    GLboolean mWriteMask;
    GLenum mComparison;
    GLboolean mStencilEnable;
    GLuint mStencilReadMask;
    GLuint mStencilWriteMask;
    Face mFrontFace;
    Face mBackFace;
    GLuint mReference;

    // Conversions from Engine values to GL4 values.
    static GLboolean const msWriteMask[];
    static GLenum const msComparison[];
    static GLenum const msOperation[];
};

inline DepthStencilState* GL4DepthStencilState::GetDepthStencilState()
{
    return static_cast<DepthStencilState*>(mGObject);
}

#endif