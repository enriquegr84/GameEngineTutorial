// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef NODEANIMATORFOLLOWCAMERA_H
#define NODEANIMATORFOLLOWCAMERA_H

#include "Graphic/Scene/Hierarchy/Node.h"

class NodeAnimatorFollowCamera : public NodeAnimator
{

public:

	//! constructor
	NodeAnimatorFollowCamera(float rotateSpeed = 0.3f, float moveSpeed = 0.5f);

	//! animates a scene node
	virtual void AnimateNode(Scene* pScene, Node* node, unsigned int timeMs);

	//! Returns the speed of movement in units per second
	float GetMoveSpeed() const;

	//! Sets the speed of movement in units per second
	void SetMoveSpeed(float moveSpeed);

	//! Returns the rotation speed
	float GetRotateSpeed() const;

	//! Set the rotation speed
	void SetRotateSpeed(float rotateSpeed);

	//! Returns type of the scene node animator
	virtual NodeAnimatorType GetType() const { return NAT_FOLLOW_CAMERA; }

	//! Creates a clone of this animator.
	virtual NodeAnimator* CreateClone(Node* node);

private:
	float mMaxVerticalAngle;

	float mMoveSpeed;
	float mRotateSpeed;

	int mLastAnimationTime;
	Vector3<float> mLastTargetPosition;
	Vector2<float> mCenterCursor, mCursorPosition;
};

#endif

