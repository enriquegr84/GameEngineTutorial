// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "NodeAnimatorRotation.h"

#include "Core/OS/OS.h"

//! constructor
NodeAnimatorRotation::NodeAnimatorRotation(unsigned int time, const Vector4<float>& rotation, float rotationSpeed)
: mRotationSpeed(rotationSpeed), mStartTime(time)
{
	mRotation = Rotation<4, float>(rotation);
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
			mRotation.mAngle += mRotationSpeed * 0.1f * diffTime * (float)GE_C_DEG_TO_RAD;
			if (mRotation.mAngle > (float)GE_C_TWO_PI) mRotation.mAngle = 0.f;
			
			Matrix4x4<float> rotation = Rotation<4, float>(mRotation);
			node->GetAbsoluteTransform().SetRotation(node->GetAbsoluteTransform().GetRotation() * rotation);

			mStartTime=timeMs;
		}
	}
}

NodeAnimator* NodeAnimatorRotation::CreateClone(Node* node)
{
	NodeAnimatorRotation* newAnimator = 
		new NodeAnimatorRotation(mStartTime, mRotation.mAxis, mRotationSpeed);

	return newAnimator;
}

