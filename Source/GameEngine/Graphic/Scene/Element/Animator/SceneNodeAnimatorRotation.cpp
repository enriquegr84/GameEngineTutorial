// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SceneNodeAnimatorRotation.h"

#include "GameEngine/GameEngine.h"
#include "OS/os.h"

//! constructor
SceneNodeAnimatorRotation::SceneNodeAnimatorRotation(unsigned int time, const Vector3<float>& rotation)
:	m_Rotation(rotation), m_StartTime(time)
{
	#ifdef _DEBUG
	//setDebugName("SceneNodeAnimatorRotation");
	#endif
}


//! animates a scene node
void SceneNodeAnimatorRotation::AnimateNode(SceneNode* node, unsigned int timeMs)
{
	if (node) // thanks to warui for this fix
	{
		const unsigned int diffTime = timeMs - m_StartTime;

		if (diffTime != 0)
		{
			// clip the rotation to small values, to avoid
			// precision problems with huge floats.
			Vector3<float> rot = node->GetRotation() + m_Rotation*(diffTime*0.1f);
			if (rot.X>360.f)
				rot.X=fmodf(rot.X, 360.f);
			if (rot.Y>360.f)
				rot.Y=fmodf(rot.Y, 360.f);
			if (rot.Z>360.f)
				rot.Z=fmodf(rot.Z, 360.f);
			node->SetRotation(rot);
			m_StartTime=timeMs;
		}
	}
}

SceneNodeAnimator* SceneNodeAnimatorRotation::CreateClone(SceneNode* node)
{
	SceneNodeAnimatorRotation* newAnimator =
		new SceneNodeAnimatorRotation(m_StartTime, m_Rotation);

	return newAnimator;
}

