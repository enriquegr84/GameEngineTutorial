// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef NODEANIMATORROTATION_H
#define NODEANIMATORROTATION_H

#include "Graphic/Scene/Hierarchy/Node.h"

class NodeAnimatorRotation : public NodeAnimator
{

public:

	//! constructor
	NodeAnimatorRotation(unsigned int time, const Vector3<float>& rotation);

	//! animates a scene node
	virtual void AnimateNode(Scene* pScene, Node* node, unsigned int timeMs);

	//! Returns type of the scene node animator
	virtual NodeAnimatorType GetType() const { return NAT_ROTATION; }

	//! Creates a clone of this animator.
	virtual NodeAnimator* CreateClone(Node* node);

protected:

	Vector3<float>	mRotation;
	unsigned int	mStartTime;
};

#endif

