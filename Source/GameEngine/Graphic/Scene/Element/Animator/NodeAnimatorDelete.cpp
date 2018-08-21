// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "NodeAnimatorDelete.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

//! constructor
NodeAnimatorDelete::NodeAnimatorDelete(unsigned int time)
: mFinishTime(time), mHasFinished(false)
{

}


//! animates a scene node
void NodeAnimatorDelete::AnimateNode(Scene* pScene, Node* node, unsigned int timeMs)
{
	if (timeMs > mFinishTime)
	{
		mHasFinished = true;

		if(node && pScene)
		{
			// don't delete if scene manager is attached to an editor
			//if (!pScene->GetParameters()->GetAttributeAsBool(_SCENE_MANAGER_IS_EDITOR))
			pScene->AddToDeletionQueue(node);
		}
	}
}


NodeAnimator* NodeAnimatorDelete::CreateClone(Node* node)
{
	NodeAnimatorDelete* newAnimator = new NodeAnimatorDelete(mFinishTime);
	return newAnimator;
}

