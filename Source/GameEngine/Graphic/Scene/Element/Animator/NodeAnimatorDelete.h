// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef NODEANIMATORDELETE_H
#define NODEANIMATORDELETE_H

#include "Graphic/Scene/Hierarchy/Node.h"

//! This is a class for animators that have a discrete end time, after which it will
//! be romved from the scene.
class NodeAnimatorDelete : public NodeAnimator
{
public:

	//! constructor
	NodeAnimatorDelete(unsigned int when);

	//! animates a scene node
	virtual void AnimateNode(Scene* pScene, Node* node, unsigned int timeMs);

	//! Returns type of the scene node animator
	virtual NodeAnimatorType GetType() const { return NAT_DELETION; }

	virtual bool HasFinished(void) const { return mHasFinished; }

	//! Creates a clone of this animator.
	virtual NodeAnimator* CreateClone(Node* node);

protected:

	unsigned int		mFinishTime;
	bool				mHasFinished;
};


#endif

