// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef NODEANIMATORTEXTURE_H
#define NODEANIMATORTEXTURE_H

#include "Graphic/Scene/Hierarchy/Node.h"

class NodeAnimatorTexture : public NodeAnimator
{
public:

	//! constructor
	NodeAnimatorTexture(const eastl::vector<Texture*>& textures,
			int timePerFrame, bool loop, unsigned int now);

	//! destructor
	virtual ~NodeAnimatorTexture();

	//! animates a scene node
	virtual void AnimateNode(Scene* pScene, Node* node, unsigned int timeMs);

	//! Returns type of the scene node animator
	virtual NodeAnimatorType GetType() const { return NAT_TEXTURE; }

	virtual bool HasFinished(void) const { return mHasFinished; }

	//! Creates a clone of this animator.
	virtual NodeAnimator* CreateClone(Node* node);

protected:

	unsigned int		mFinishTime;
	bool				mHasFinished;

private:

	void ClearTextures();

	eastl::vector<Texture*> mTextures;
	unsigned int mTimePerFrame;
	unsigned int mStartTime;
	bool mLoop;
};

#endif

