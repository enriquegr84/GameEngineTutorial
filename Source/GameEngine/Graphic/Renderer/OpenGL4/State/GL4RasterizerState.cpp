// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)


#include "Core/Logger/Logger.h"
#include "GL4RasterizerState.h"

GL4RasterizerState::GL4RasterizerState(RasterizerState const* rasterizerState)
    :
    GL4DrawingState(rasterizerState)
{
    mFillMode = msFillMode[rasterizerState->mFillMode];
    mCullFace = msCullFace[rasterizerState->mCullMode];
    mFrontFace = (rasterizerState->mFrontCCW ? GL_CCW : GL_CW);
    mDepthScale = rasterizerState->mSlopeScaledDepthBias;
    mDepthBias = static_cast<float>(rasterizerState->mDepthBias);
    mEnableScissor = (rasterizerState->mEnableScissor ? GL_TRUE : GL_FALSE);
}

eastl::shared_ptr<GraphicObject> GL4RasterizerState::Create(void*, GraphicObject const* object)
{
    if (object->GetType() == GE_RASTERIZER_STATE)
    {
        return eastl::make_shared<GL4RasterizerState>(
            static_cast<RasterizerState const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}

void GL4RasterizerState::Enable()
{
    glPolygonMode(GL_FRONT_AND_BACK, mFillMode);

    if (mCullFace != 0)
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(mFrontFace);
        glCullFace(mCullFace);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    if (mDepthScale != 0.0f && mDepthBias != 0.0f)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glEnable(GL_POLYGON_OFFSET_POINT);
        glPolygonOffset(mDepthScale, mDepthBias);
    }
    else
    {
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glDisable(GL_POLYGON_OFFSET_POINT);
    }
}


GLenum const GL4RasterizerState::msFillMode[] =
{
    GL_FILL,
    GL_LINE
};

GLenum const GL4RasterizerState::msCullFace[] =
{
    0,
    GL_FRONT,
    GL_BACK
};
