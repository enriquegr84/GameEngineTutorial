// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef GL4DRAWTARGET_H
#define GL4DRAWTARGET_H

#include "Graphic/Resource/Texture/DrawTarget.h"
#include "GL4TextureDS.h"
#include "GL4TextureRT.h"

class GRAPHIC_ITEM GL4DrawTarget : public DrawTarget
{
public:
    // Construction and destruction.
    virtual ~GL4DrawTarget();
    GL4DrawTarget(DrawTarget const* target,
        eastl::vector<GL4TextureRT*>& rtTextures, GL4TextureDS* dsTexture);
    static eastl::shared_ptr<DrawTarget> Create(DrawTarget const* target,
        eastl::vector<GraphicObject*>& rtTextures, GraphicObject* dsTexture);

    // Member access.
	inline DrawTarget* GetDrawTarget() const;
    inline GL4TextureRT* GetRTTexture(unsigned int i) const;
    inline GL4TextureDS* GetDSTexture() const;

    // Used in the Renderer::Draw function.
    void Enable();
    void Disable();

protected:
	DrawTarget * mTarget;

private:
    eastl::vector<GL4TextureRT*> mRTTextures;
    GL4TextureDS* mDSTexture;

    GLuint mFrameBuffer;

    // Temporary storage during enable/disable of targets.
    GLint mSaveViewportX;
    GLint mSaveViewportY;
    GLsizei mSaveViewportWidth;
    GLsizei mSaveViewportHeight;
    GLdouble mSaveViewportNear;
    GLdouble mSaveViewportFar;
};

inline DrawTarget* GL4DrawTarget::GetDrawTarget() const
{
	return mTarget;
}

inline GL4TextureRT* GL4DrawTarget::GetRTTexture(unsigned int i) const
{
    return mRTTextures[i];
}

inline GL4TextureDS* GL4DrawTarget::GetDSTexture() const
{
    return mDSTexture;
}

#endif