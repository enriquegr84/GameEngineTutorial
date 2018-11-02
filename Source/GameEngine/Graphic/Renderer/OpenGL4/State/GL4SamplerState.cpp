// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Core/Logger/Logger.h"
#include "GL4SamplerState.h"

GL4SamplerState::~GL4SamplerState()
{
    glDeleteSamplers(1, &mGLHandle);
}

GL4SamplerState::GL4SamplerState(SamplerState const* samplerState)
    :
    GL4DrawingState(samplerState)
{
    glGenSamplers(1, &mGLHandle);

    glSamplerParameteri(mGLHandle, GL_TEXTURE_WRAP_S, msMode[samplerState->mMode[0]]);
    glSamplerParameteri(mGLHandle, GL_TEXTURE_WRAP_T, msMode[samplerState->mMode[1]]);
    glSamplerParameteri(mGLHandle, GL_TEXTURE_WRAP_R, msMode[samplerState->mMode[2]]);

    // TODO - GL_TEXTURE_MAX_ANISOTROPY_EXT is not defined?
    // glSamplerParameterf(samplerState, GL_TEXTURE_MAX_ANISOTROPY_EXT, samplerState->maxAnisotropy);

    glSamplerParameterf(mGLHandle, GL_TEXTURE_MIN_LOD, samplerState->mMinLOD);
    glSamplerParameterf(mGLHandle, GL_TEXTURE_MAX_LOD, samplerState->mMaxLOD);
    glSamplerParameterf(mGLHandle, GL_TEXTURE_LOD_BIAS, samplerState->mMipLODBias);

    float borderColor[4];
    borderColor[0] = samplerState->mBorderColor[0];
    borderColor[1] = samplerState->mBorderColor[1];
    borderColor[2] = samplerState->mBorderColor[2];
    borderColor[3] = samplerState->mBorderColor[3];
    glSamplerParameterfv(mGLHandle, GL_TEXTURE_BORDER_COLOR, borderColor);

    switch(samplerState->mFilter)
    {
        case SamplerState::MIN_P_MAG_P_MIP_P:
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerState::MIN_P_MAG_P_MIP_L:
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerState::MIN_P_MAG_L_MIP_P:
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerState::MIN_P_MAG_L_MIP_L:
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerState::MIN_L_MAG_P_MIP_P:
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerState::MIN_L_MAG_P_MIP_L:
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerState::MIN_L_MAG_L_MIP_P:
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerState::MIN_L_MAG_L_MIP_L:
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        default:
            LogWarning("GL4 does not support samplerState filter = " + samplerState->mFilter);
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, 0);
            glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, 0);
            break;
    }
}

eastl::shared_ptr<GraphicObject> GL4SamplerState::Create(void*, GraphicObject const* object)
{
    if (object->GetType() == GE_SAMPLER_STATE)
    {
        return eastl::make_shared<GL4SamplerState>(
            static_cast<SamplerState const*>(object));
    }

    LogError("Invalid object type.");
    return nullptr;
}


GLint const GL4SamplerState::msMode[] =
{
    GL_REPEAT,          // WRAP
    GL_MIRRORED_REPEAT, // MIRROR
    GL_CLAMP_TO_EDGE,   // CLAMP
    GL_CLAMP_TO_BORDER, // BORDER
    GL_MIRRORED_REPEAT  // MIRROR_ONCE
};
