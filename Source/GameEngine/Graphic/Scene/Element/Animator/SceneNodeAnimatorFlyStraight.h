// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _SCENENODEANIMATORFLYSTRAIGHT_H_INCLUDED_
#define _SCENENODEANIMATORFLYSTRAIGHT_H_INCLUDED_

#include "Scenes/SceneNodes.h"


class SceneNodeAnimatorFlyStraight : public SceneNodeAnimator
{
public:

	//! constructor
	SceneNodeAnimatorFlyStraight(const Vector3<float>& startPoint, const Vector3<float>& endPoint,
									unsigned int timeForWay, bool loop, unsigned int now, bool pingpong = false);

	//! animates a scene node
	virtual void AnimateNode(SceneNode* node, unsigned int timeMs);

	//! Returns type of the scene node animator
	virtual E_SCENE_NODE_ANIMATOR_TYPE GetType() const { return ESNAT_FLY_STRAIGHT; }
	
	virtual bool HasFinished(void) const { return m_HasFinished; }
	
	//! Creates a clone of this animator.
	virtual SceneNodeAnimator* CreateClone(SceneNode* node);

protected:

	unsigned int		m_FinishTime;
	bool	m_HasFinished;

private:

	void RecalculateIntermediateValues();

	Vector3<float> m_Start;
	Vector3<float> m_End;
	Vector3<float> m_Vector;
	float m_TimeFactor;
	unsigned int m_StartTime;
	unsigned int m_TimeForWay;
	bool m_Loop;
	bool m_PingPong;
};

#endif

