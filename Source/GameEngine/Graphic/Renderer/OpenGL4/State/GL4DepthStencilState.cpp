// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)


#include "Core/Logger/Logger.h"
#include "GL4DepthStencilState.h"


GL4DepthStencilState::GL4DepthStencilState(DepthStencilState const* depthStencilState)
    :
    GL4DrawingState(depthStencilState)
{
    mDepthEnable = (depthStencilState->mDepthEnable ? GL_TRUE : GL_FALSE);
    mWriteMask = msWriteMask[depthStencilState->mWriteMask];
    mComparison = msComparison[depthStencilState->mComparison];
    mStencilEnable = (depthStencilState->mStencilEnable ? GL_TRUE : GL_FALSE);
    mStencilReadMask = depthStencilState->mStencilReadMask;
    mStencilWriteMask = depthStencilState->mStencilWriteMask;
    DepthStencilState::Face front = depthStencilState->mFrontFace;
    mFrontFace.onFail = msOperation[front.fail];
    mFrontFace.onZFail = msOperation[front.depthFail];
    mFrontFace.onZPass = msOperation[front.pass];
    mFrontFace.comparison = msComparison[front.comparison];
    DepthStencilState::Face back = depthStencilState->mBackFace;
    mBackFace.onFail = msOperation[back.fail];
    mBackFace.onZFail = msOperation[back.depthFail];
    mBackFace.onZPass = msOperation[back.pass];
    mBackFace.comparison = msComparison[back.comparison];
    mReference = depthStencilState->mReference;
}

eastl::shared_ptr<GraphicObject> GL4DepthStencilState::Create(void*, GraphicObject const* object)
{
    if (object->GetType() == GE_DEPTH_STENCIL_STATE)
    {
        return eastl::make_shared<GL4DepthStencilState>(
            static_cast<DepthStencilState const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}

void GL4DepthStencilState::Enable()
{
    if (mDepthEnable)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(mComparison);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    glDepthMask(mWriteMask);

    if (mStencilEnable)
    {
        glEnable(GL_STENCIL_TEST);

        glStencilFuncSeparate(GL_FRONT, mFrontFace.comparison, mReference, mStencilReadMask);
        glStencilMaskSeparate(GL_FRONT, mStencilWriteMask);
        glStencilOpSeparate(GL_FRONT, mFrontFace.onFail, mFrontFace.onZFail, mFrontFace.onZPass);
        glStencilFuncSeparate(GL_BACK, mBackFace.comparison, mReference, mStencilReadMask);
        glStencilMaskSeparate(GL_BACK, mStencilWriteMask);
        glStencilOpSeparate(GL_BACK, mBackFace.onFail, mBackFace.onZFail, mBackFace.onZPass);
    }
    else
    {
        glDisable(GL_STENCIL_TEST);
    }
}


GLboolean const GL4DepthStencilState::msWriteMask[] =
{
    GL_FALSE,
    GL_TRUE
};

GLenum const GL4DepthStencilState::msComparison[] =
{
    GL_NEVER,
    GL_LESS,
    GL_EQUAL,
    GL_LEQUAL,
    GL_GREATER,
    GL_NOTEQUAL,
    GL_GEQUAL,
    GL_ALWAYS
};

GLenum const GL4DepthStencilState::msOperation[] =
{
    GL_KEEP,
    GL_ZERO,
    GL_REPLACE,
    GL_INCR,  // TODO: DX11 has INCR_SAT that clamps the result
    GL_DECR,  // TODO: DX11 has INCR_SAT that clamps the result
    GL_INVERT,
    GL_INCR,
    GL_DECR
};
