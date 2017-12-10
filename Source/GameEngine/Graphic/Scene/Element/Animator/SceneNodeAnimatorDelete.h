// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _SCENENODEANIMATORDELETE_H_INCLUDED_
#define _SCENENODEANIMATORDELETE_H_INCLUDED_

#include "Scenes/SceneNodes.h"

//! This is a class for animators that have a discrete end time, after which it will
//! be romved from the scene.
class SceneNodeAnimatorDelete : public SceneNodeAnimator
{
public:

	//! constructor
	SceneNodeAnimatorDelete(unsigned int when);

	//! animates a scene node
	virtual void AnimateNode(SceneNode* node, unsigned int timeMs);

	//! Returns type of the scene node animator
	virtual E_SCENE_NODE_ANIMATOR_TYPE GetType() const { return ESNAT_DELETION; }

	virtual bool HasFinished(void) const { return m_HasFinished; }

	//! Creates a clone of this animator.
	virtual SceneNodeAnimator* CreateClone(SceneNode* node);

protected:

	unsigned int		m_FinishTime;
	bool	m_HasFinished;
};


#endif

