// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "NodeAnimatorTexture.h"

#include "Core/OS/OS.h"

//! constructor
NodeAnimatorTexture::NodeAnimatorTexture(
	const eastl::vector<eastl::shared_ptr<Texture2>>& textures, 
	int timePerFrame, bool loop, unsigned int now)
: mFinishTime(0), mHasFinished(false), mTimePerFrame(timePerFrame), mStartTime(now), mLoop(loop)
{
	for (unsigned int i=0; i<textures.size(); ++i)
		mTextures.push_back(textures[i]);

	mFinishTime = now + (timePerFrame * mTextures.size());
}


//! destructor
NodeAnimatorTexture::~NodeAnimatorTexture()
{
	ClearTextures();
}


void NodeAnimatorTexture::ClearTextures()
{
	for (unsigned int i=0; i<mTextures.size(); ++i)
		if (mTextures[i])
			mTextures[i] = 0;

	mTextures.clear();
}


//! animates a scene node
void NodeAnimatorTexture::AnimateNode(Scene* pScene, Node* node, unsigned int timeMs)
{
	if(!node)
		return;

	if (mTextures.size())
	{
		const unsigned int t = (timeMs-mStartTime);

		unsigned int idx = 0;
		if (!mLoop && timeMs >= mFinishTime)
		{
			idx = mTextures.size() - 1;
			mHasFinished = true;
		}
		else
		{
			idx = (t/mTimePerFrame) % mTextures.size();
		}

		if (idx < mTextures.size())
			if (node->GetMaterial(0)->GetTexture(0) != mTextures[idx])
				node->SetMaterialTexture(0, mTextures[idx]);
	}
}


NodeAnimator* NodeAnimatorTexture::CreateClone(Node* node)
{
	NodeAnimatorTexture * newAnimator =
		new NodeAnimatorTexture(mTextures, mTimePerFrame, mLoop, mStartTime);

	return newAnimator;
}

