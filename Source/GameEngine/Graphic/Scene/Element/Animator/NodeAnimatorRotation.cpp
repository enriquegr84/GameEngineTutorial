// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "NodeAnimatorRotation.h"

#include "Core/OS/OS.h"

//! constructor
NodeAnimatorRotation::NodeAnimatorRotation(unsigned int time, const Vector3<float>& rotation)
:	mRotation(rotation), mStartTime(time)
{

}


//! animates a scene node
void NodeAnimatorRotation::AnimateNode(Scene* pScene, Node* node, unsigned int timeMs)
{
	if (node) // thanks to warui for this fix
	{
		const unsigned int diffTime = timeMs - mStartTime;

		if (diffTime != 0)
		{
			// clip the rotation to small values, to avoid
			// precision problems with huge floats.
			/*
			Vector3<float> rot = node->GetAbsoluteTransform().GetRotation() + mRotation*(diffTime*0.1f);
			if (rot.X>360.f)
				rot.X=fmodf(rot.X, 360.f);
			if (rot.Y>360.f)
				rot.Y=fmodf(rot.Y, 360.f);
			if (rot.Z>360.f)
				rot.Z=fmodf(rot.Z, 360.f);
			node->SetRotation(rot);
			*/
			mStartTime=timeMs;
		}
	}
}

NodeAnimator* NodeAnimatorRotation::CreateClone(Node* node)
{
	NodeAnimatorRotation* newAnimator = 
		new NodeAnimatorRotation(mStartTime, mRotation);

	return newAnimator;
}

