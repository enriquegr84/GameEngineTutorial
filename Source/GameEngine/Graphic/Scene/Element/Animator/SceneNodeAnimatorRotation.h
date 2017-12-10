// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _SCENENODEANIMATORROTATION_H_INCLUDED_
#define _SCENENODEANIMATORROTATION_H_INCLUDED_

#include "Scenes/SceneNodes.h"

class SceneNodeAnimatorRotation : public SceneNodeAnimator
{

public:

	//! constructor
	SceneNodeAnimatorRotation(unsigned int time, const Vector3<float>& rotation);

	//! animates a scene node
	virtual void AnimateNode(SceneNode* node, unsigned int timeMs);

	//! Returns type of the scene node animator
	virtual E_SCENE_NODE_ANIMATOR_TYPE GetType() const { return ESNAT_ROTATION; }

	//! Creates a clone of this animator.
	virtual SceneNodeAnimator* CreateClone(SceneNode* node);

protected:

	Vector3<float>	m_Rotation;
	unsigned int			m_StartTime;
};

#endif

