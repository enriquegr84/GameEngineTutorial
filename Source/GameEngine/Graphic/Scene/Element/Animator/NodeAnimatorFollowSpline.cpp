// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "NodeAnimatorFollowSpline.h"

//! constructor
NodeAnimatorFollowSpline::NodeAnimatorFollowSpline(unsigned int time, const eastl::vector<Vector3<float>>& points,  
		float speed, float tightness, bool loop, bool pingpong)
:	mFinishTime(0), mHasFinished(false), mPoints(points), mSpeed(speed), 
	mTightness(tightness), mStartTime(time), mLoop(loop), mPingPong(pingpong)
{
	#ifdef _DEBUG
	//setDebugName("SceneNodeAnimatorFollowSpline");
	#endif
}


inline int NodeAnimatorFollowSpline::Clamp(int idx, int size)
{
	return ( idx<0 ? size+idx : ( idx>=size ? idx-size : idx ) );
}


//! animates a scene node
void NodeAnimatorFollowSpline::AnimateNode(Node* node, unsigned int timeMs)
{
	if(!node)
		return;

	const unsigned int pSize = mPoints.size();
	if (pSize==0)
	{
		if ( !mLoop )
			mHasFinished = true;
		return;
	}
	if (pSize==1)
	{
		if ( timeMs > mStartTime )
		{
			node->SetPosition(mPoints[0]);
			if ( !mLoop )
				mHasFinished = true;
		}
		return;
	}

	const float dt = ( (timeMs-mStartTime) * mSpeed * 0.001f );
	const int unwrappedIdx = floor32( dt );
	if ( !mLoop && unwrappedIdx >= (int)pSize-1 )
	{
		node->SetPosition(mPoints[pSize-1]);
		mHasFinished = true;
		return;
	}
	const bool pong = mPingPong && (unwrappedIdx/(pSize-1))%2;
	const float u =  pong ? 1.f-fract ( dt ) : fract ( dt );
	const int idx = pong ?	(pSize-2) - (unwrappedIdx % (pSize-1))
						: (mPingPong ? unwrappedIdx % (pSize-1) 
							: unwrappedIdx % pSize);
	//const float u = 0.001f * fmodf( dt, 1000.0f );

	const Vector3<float>& p0 = mPoints[ Clamp( idx - 1, pSize ) ];
	const Vector3<float>& p1 = mPoints[ Clamp( idx + 0, pSize ) ]; // starting point
	const Vector3<float>& p2 = mPoints[ Clamp( idx + 1, pSize ) ]; // end point
	const Vector3<float>& p3 = mPoints[ Clamp( idx + 2, pSize ) ];

	// hermite polynomials
	const float h1 = 2.0f * u * u * u - 3.0f * u * u + 1.0f;
	const float h2 = -2.0f * u * u * u + 3.0f * u * u;
	const float h3 = u * u * u - 2.0f * u * u + u;
	const float h4 = u * u * u - u * u;

	// tangents
	const Vector3<float> t1 = ( p2 - p0 ) * mTightness;
	const Vector3<float> t2 = ( p3 - p1 ) * mTightness;

	// interpolated point
	node->SetPosition(p1 * h1 + p2 * h2 + t1 * h3 + t2 * h4);
}


NodeAnimator* NodeAnimatorFollowSpline::CreateClone(Node* node)
{
	NodeAnimatorFollowSpline * newAnimator =
		new NodeAnimatorFollowSpline(mStartTime, mPoints, mSpeed, mTightness);

	return newAnimator;
}

