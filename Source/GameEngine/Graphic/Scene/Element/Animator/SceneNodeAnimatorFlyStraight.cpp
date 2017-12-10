// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SceneNodeAnimatorFlyStraight.h"

//! constructor
SceneNodeAnimatorFlyStraight::SceneNodeAnimatorFlyStraight(const Vector3<float>& startPoint,
			const Vector3<float>& endPoint, unsigned int timeForWay, bool loop, unsigned int now, bool pingpong)
:	m_FinishTime(now + timeForWay), m_HasFinished(false), m_Start(startPoint), m_End(endPoint), 
	m_TimeFactor(0.0f), m_StartTime(now), m_TimeForWay(timeForWay), m_Loop(loop), m_PingPong(pingpong)
{
	#ifdef _DEBUG
	//setDebugName("SceneNodeAnimatorFlyStraight");
	#endif

	RecalculateIntermediateValues();
}


void SceneNodeAnimatorFlyStraight::RecalculateIntermediateValues()
{
	m_Vector = m_End - m_Start;
	m_TimeFactor = (float)m_Vector.GetLength() / m_TimeForWay;
	m_Vector.Normalize();
}


//! animates a scene node
void SceneNodeAnimatorFlyStraight::AnimateNode(SceneNode* node, unsigned int timeMs)
{
	if (!node)
		return;

	unsigned int t = (timeMs-m_StartTime);

	Vector3<float> pos;

	if (!m_Loop && !m_PingPong && t >= m_TimeForWay)
	{
		pos = m_End;
		m_HasFinished = true;
	}
	else if (!m_Loop && m_PingPong && t >= m_TimeForWay * 2.f )
	{
		pos = m_Start;
		m_HasFinished = true;
	}
	else
	{
		float phase = fmodf( (float) t, (float) m_TimeForWay );
		Vector3<float> rel = m_Vector * phase * m_TimeFactor;
		const bool pong = m_PingPong && fmodf( (float) t, (float) m_TimeForWay*2.f ) >= m_TimeForWay;

		if ( !pong )
		{
			pos += m_Start + rel;
		}
		else
		{
			pos = m_End - rel;
		}
	}

	node->SetPosition(pos);
}

SceneNodeAnimator* SceneNodeAnimatorFlyStraight::CreateClone(SceneNode* node)
{
	SceneNodeAnimatorFlyStraight * newAnimator =
		new SceneNodeAnimatorFlyStraight(m_Start, m_End, m_TimeForWay, m_Loop, m_StartTime, m_PingPong);

	return newAnimator;
}

