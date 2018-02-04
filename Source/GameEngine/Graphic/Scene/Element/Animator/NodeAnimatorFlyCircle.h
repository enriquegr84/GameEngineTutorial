// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef NODEANIMATORFLYCIRCLE_H
#define NODEANIMATORFLYCIRCLE_H

#include "Graphic/Scene/Hierarchy/Node.h"

class NodeAnimatorFlyCircle : public NodeAnimator
{

public:

	//! constructor
	NodeAnimatorFlyCircle(unsigned int time, const Vector3<float>& center, float radius,
					float speed, const Vector3<float>& direction, float radiusEllipsoid);

	//! animates a scene node
	virtual void AnimateNode(Scene* pScene, Node* node, unsigned int timeMs);

	//! Returns type of the scene node animator
	virtual NodeAnimatorType GetType() const { return NAT_FLY_CIRCLE; }

	//! Creates a clone of this animator.
	virtual NodeAnimator* CreateClone(Node* node);

private:
	// do some initial calculations
	void Init();

	// circle center
	Vector3<float> mCenter;
	// up-vector, normal to the circle's plane
	Vector3<float> mDirection;
	// Two helper vectors
	Vector3<float> mVecU;
	Vector3<float> mVecV;
	float mRadius;
	float mRadiusEllipsoid;
	float mSpeed;
	unsigned int mStartTime;
};

#endif

