// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef NODEANIMATORFOLLOWSPLINE_H
#define NODEANIMATORFOLLOWSPLINE_H

#include "Graphic/Scene/Hierarchy/Node.h"


//! Scene node animator based free code Matthias Gall wrote and sent in. (Most of
//! this code is written by him, I only modified bits.)
class NodeAnimatorFollowSpline : public NodeAnimator
{
public:

	//! constructor
	NodeAnimatorFollowSpline(unsigned int startTime, const eastl::vector<Vector3<float>>& points,
			float speed = 1.0f, float tightness = 0.5f, bool loop=true, bool pingpong=false);

	//! animates a scene node
	virtual void AnimateNode(Node* node, unsigned int timeMs);

	//! Returns type of the scene node animator
	virtual E_SCENE_NODE_ANIMATOR_TYPE GetType() const { return ESNAT_FOLLOW_SPLINE; }

	virtual bool HasFinished(void) const { return mHasFinished; }

	//! Creates a clone of this animator.
	virtual NodeAnimator* CreateClone(Node* node);

protected:

	unsigned int		mFinishTime;
	bool				mHasFinished;

	//! clamps a the value idx to fit into range 0..size-1
	int Clamp(int idx, int size);

	eastl::vector<Vector3<float>> mPoints;
	float mSpeed;
	float mTightness;
	unsigned int mStartTime;
	bool mLoop;
	bool mPingPong;
};

#endif

