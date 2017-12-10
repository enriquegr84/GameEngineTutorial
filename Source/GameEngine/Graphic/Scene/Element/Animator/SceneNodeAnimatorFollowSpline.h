// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _SCENENODEANIMATORFOLLOWSPLINE_H_INCLUDED_
#define _SCENENODEANIMATORFOLLOWSPLINE_H_INCLUDED_

#include "Scenes/SceneNodes.h"


//! Scene node animator based free code Matthias Gall wrote and sent in. (Most of
//! this code is written by him, I only modified bits.)
class SceneNodeAnimatorFollowSpline : public SceneNodeAnimator
{
public:

	//! constructor
	SceneNodeAnimatorFollowSpline(unsigned int startTime, const eastl::vector< Vector3<float> >& points,
			float speed = 1.0f, float tightness = 0.5f, bool loop=true, bool pingpong=false);

	//! animates a scene node
	virtual void AnimateNode(SceneNode* node, unsigned int timeMs);

	//! Returns type of the scene node animator
	virtual E_SCENE_NODE_ANIMATOR_TYPE GetType() const { return ESNAT_FOLLOW_SPLINE; }

	virtual bool HasFinished(void) const { return m_HasFinished; }

	//! Creates a clone of this animator.
	virtual SceneNodeAnimator* CreateClone(SceneNode* node);

protected:

	unsigned int		m_FinishTime;
	bool	m_HasFinished;

	//! clamps a the value idx to fit into range 0..size-1
	int Clamp(int idx, int size);

	eastl::vector< Vector3<float> > m_Points;
	float m_Speed;
	float m_Tightness;
	unsigned int m_StartTime;
	bool m_Loop;
	bool m_PingPong;
};

#endif

