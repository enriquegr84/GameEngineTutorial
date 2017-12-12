// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "NodeAnimatorFlyCircle.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

//! constructor
NodeAnimatorFlyCircle::NodeAnimatorFlyCircle(unsigned int time, const Vector3<float>& center, 
	float radius, float speed, const Vector3<float>& direction, float radiusEllipsoid)
:	mCenter(center), mDirection(direction),  mRadius(radius), 
	mRadiusEllipsoid(radiusEllipsoid), mSpeed(speed), mStartTime(time)
{
	#ifdef _DEBUG
	//setDebugName("SceneNodeAnimatorFlyCircle");
	#endif
	Init();
}


void NodeAnimatorFlyCircle::Init()
{
	mDirection.Normalize();

	if (mDirection.Y != 0)
		mVecV = Vector3<float>(50,0,0).CrossProduct(mDirection).Normalize();
	else
		mVecV = Vector3<float>(0,50,0).CrossProduct(mDirection).Normalize();
	mVecU = mVecV.CrossProduct(mDirection).Normalize();
}


//! animates a scene node
void NodeAnimatorFlyCircle::AnimateNode(Node* node, unsigned int timeMs)
{
	if ( 0 == node )
		return;

	float time;

	// Check for the condition where the StartTime is in the future.
	if(mStartTime > timeMs)
		time = ((int)timeMs - (int)mStartTime) * mSpeed;
	else
		time = (timeMs - mStartTime) * mSpeed;

//	node->SetPosition(mCenter + Radius * ((mVecU*cosf(time)) + (mVecV*sinf(time))));
	float r2 = mRadiusEllipsoid == 0.f ? mRadius : mRadiusEllipsoid;
	node->SetPosition(mCenter + (mRadius*cosf(time)*mVecU) + (r2*sinf(time)*mVecV ) );
}


NodeAnimator* NodeAnimatorFlyCircle::CreateClone(Node* node)
{
	NodeAnimatorFlyCircle* newAnimator =
		new NodeAnimatorFlyCircle(mStartTime, mCenter, mRadius, mSpeed, mDirection, mRadiusEllipsoid);

	return newAnimator;
}