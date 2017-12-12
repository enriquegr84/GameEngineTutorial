// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef NODEANIMATORFLYSTRAIGHT_H
#define NODEANIMATORFLYSTRAIGHT_H

#include "Graphic/Scene/Hierarchy/Node.h"

class NodeAnimatorFlyStraight : public NodeAnimator
{
public:

	//! constructor
	NodeAnimatorFlyStraight(const Vector3<float>& startPoint, const Vector3<float>& endPoint,
					unsigned int timeForWay, bool loop, unsigned int now, bool pingpong = false);

	//! animates a scene node
	virtual void AnimateNode(Node* node, unsigned int timeMs);

	//! Returns type of the scene node animator
	virtual E_SCENE_NODE_ANIMATOR_TYPE GetType() const { return ESNAT_FLY_STRAIGHT; }
	
	virtual bool HasFinished(void) const { return mHasFinished; }
	
	//! Creates a clone of this animator.
	virtual NodeAnimator* CreateClone(Node* node);

protected:

	unsigned int		mFinishTime;
	bool				mHasFinished;

private:

	void RecalculateIntermediateValues();

	Vector3<float> mStart;
	Vector3<float> mEnd;
	Vector3<float> mVector;
	float mTimeFactor;
	unsigned int mStartTime;
	unsigned int mTimeForWay;
	bool mLoop;
	bool mPingPong;
};

#endif

