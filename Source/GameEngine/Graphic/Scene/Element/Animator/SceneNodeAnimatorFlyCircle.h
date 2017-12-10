// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _SCENENODEANIMATORFLYCIRCLE_H_INCLUDED_
#define _SCENENODEANIMATORFLYCIRCLE_H_INCLUDED_

#include "Scenes/SceneNodes.h"


class SceneNodeAnimatorFlyCircle : public SceneNodeAnimator
{

public:

	//! constructor
	SceneNodeAnimatorFlyCircle(unsigned int time, const Vector3<float>& center, float radius,
					float speed, const Vector3<float>& direction, float radiusEllipsoid);

	//! animates a scene node
	virtual void AnimateNode(SceneNode* node, unsigned int timeMs);

	//! Returns type of the scene node animator
	virtual E_SCENE_NODE_ANIMATOR_TYPE getType() const { return ESNAT_FLY_CIRCLE; }

	//! Creates a clone of this animator.
	virtual SceneNodeAnimator* CreateClone(SceneNode* node);

private:
	// do some initial calculations
	void Init();

	// circle center
	Vector3<float> m_Center;
	// up-vector, normal to the circle's plane
	Vector3<float> m_Direction;
	// Two helper vectors
	Vector3<float> m_VecU;
	Vector3<float> m_VecV;
	float m_Radius;
	float m_RadiusEllipsoid;
	float m_Speed;
	unsigned int m_StartTime;
};

#endif

