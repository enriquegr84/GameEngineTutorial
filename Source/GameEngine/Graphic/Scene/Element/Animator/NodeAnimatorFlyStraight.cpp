// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "NodeAnimatorFlyStraight.h"

//! constructor
NodeAnimatorFlyStraight::NodeAnimatorFlyStraight(const Vector3<float>& startPoint,
			const Vector3<float>& endPoint, unsigned int timeForWay, bool loop, 
			unsigned int now, bool pingpong)
:	mFinishTime(now + timeForWay), mHasFinished(false), mStart(startPoint), mEnd(endPoint), 
	mTimeFactor(0.0f), mStartTime(now), mTimeForWay(timeForWay), mLoop(loop), mPingPong(pingpong)
{
	RecalculateIntermediateValues();
}


void NodeAnimatorFlyStraight::RecalculateIntermediateValues()
{
	mVector = mEnd - mStart;
	mTimeFactor = Length(mVector) / (float)mTimeForWay;
	Normalize(mVector);
}


//! animates a scene node
void NodeAnimatorFlyStraight::AnimateNode(Scene* pScene, Node* node, unsigned int timeMs)
{
	if (!node)
		return;

	unsigned int t = (timeMs-mStartTime);

	Vector3<float> pos = Vector3<float>::Zero();

	if (!mLoop && !mPingPong && t >= mTimeForWay)
	{
		pos = mEnd;
		mHasFinished = true;
	}
	else if (!mLoop && mPingPong && t >= mTimeForWay * 2.f )
	{
		pos = mStart;
		mHasFinished = true;
	}
	else
	{
		float phase = fmodf( (float) t, (float) mTimeForWay );
		Vector3<float> rel = mVector * phase * mTimeFactor;
		const bool pong = mPingPong && fmodf( (float) t, (float) mTimeForWay*2.f ) >= mTimeForWay;

		if ( !pong )
		{
			pos += mStart + rel;
		}
		else
		{
			pos = mEnd - rel;
		}
	}

	node->GetAbsoluteTransform().SetTranslation(pos);
}

NodeAnimator* NodeAnimatorFlyStraight::CreateClone(Node* node)
{
	NodeAnimatorFlyStraight * newAnimator =
		new NodeAnimatorFlyStraight(mStart, mEnd, mTimeForWay, mLoop, mStartTime, mPingPong);

	return newAnimator;
}

