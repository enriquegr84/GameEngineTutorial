// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SceneNodeAnimatorDelete.h"

#include "GameEngine/GameEngine.h"
#include "OS/os.h"

#include "Scenes/Scene.h"

//! constructor
SceneNodeAnimatorDelete::SceneNodeAnimatorDelete(unsigned int time)
: m_FinishTime(time), m_HasFinished(false)
{
	#ifdef _DEBUG
	//setDebugName("CSceneNodeAnimatorDelete");
	#endif
}


//! animates a scene node
void SceneNodeAnimatorDelete::AnimateNode(SceneNode* node, unsigned int timeMs)
{
	if (timeMs > m_FinishTime)
	{
		m_HasFinished = true;
		shared_ptr<ScreenElementScene> pScene = g_pGameApp->GetHumanView()->m_pScene;

		if(node && pScene)
		{
			// don't delete if scene manager is attached to an editor
			//if (!pScene->GetParameters()->GetAttributeAsBool(_SCENE_MANAGER_IS_EDITOR))
				pScene->AddToDeletionQueue(node);
		}
	}
}


SceneNodeAnimator* SceneNodeAnimatorDelete::CreateClone(SceneNode* node)
{
	SceneNodeAnimatorDelete* newAnimator = new SceneNodeAnimatorDelete(m_FinishTime);
	return newAnimator;
}

