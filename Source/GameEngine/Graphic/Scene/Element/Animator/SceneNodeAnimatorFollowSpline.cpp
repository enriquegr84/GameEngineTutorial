// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SceneNodeAnimatorFollowSpline.h"


//! constructor
SceneNodeAnimatorFollowSpline::SceneNodeAnimatorFollowSpline(unsigned int time, const eastl::vector<Vector3<float>>& points,  
		float speed, float tightness, bool loop, bool pingpong)
:	m_FinishTime(0), m_HasFinished(false), m_Points(points), m_Speed(speed), 
	m_Tightness(tightness), m_StartTime(time), m_Loop(loop), m_PingPong(pingpong)
{
	#ifdef _DEBUG
	//setDebugName("SceneNodeAnimatorFollowSpline");
	#endif
}


inline int SceneNodeAnimatorFollowSpline::Clamp(int idx, int size)
{
	return ( idx<0 ? size+idx : ( idx>=size ? idx-size : idx ) );
}


//! animates a scene node
void SceneNodeAnimatorFollowSpline::AnimateNode(SceneNode* node, unsigned int timeMs)
{
	if(!node)
		return;

	const unsigned int pSize = m_Points.size();
	if (pSize==0)
	{
		if ( !m_Loop )
			m_HasFinished = true;
		return;
	}
	if (pSize==1)
	{
		if ( timeMs > m_StartTime )
		{
			node->SetPosition(m_Points[0]);
			if ( !m_Loop )
				m_HasFinished = true;
		}
		return;
	}

	const float dt = ( (timeMs-m_StartTime) * m_Speed * 0.001f );
	const int unwrappedIdx = floor32( dt );
	if ( !m_Loop && unwrappedIdx >= (int)pSize-1 )
	{
		node->SetPosition(m_Points[pSize-1]);
		m_HasFinished = true;
		return;
	}
	const bool pong = m_PingPong && (unwrappedIdx/(pSize-1))%2;
	const float u =  pong ? 1.f-fract ( dt ) : fract ( dt );
	const int idx = pong ?	(pSize-2) - (unwrappedIdx % (pSize-1))
						: (m_PingPong ? unwrappedIdx % (pSize-1)
									: unwrappedIdx % pSize);
	//const float u = 0.001f * fmodf( dt, 1000.0f );

	const Vector3<float>& p0 = m_Points[ Clamp( idx - 1, pSize ) ];
	const Vector3<float>& p1 = m_Points[ Clamp( idx + 0, pSize ) ]; // starting point
	const Vector3<float>& p2 = m_Points[ Clamp( idx + 1, pSize ) ]; // end point
	const Vector3<float>& p3 = m_Points[ Clamp( idx + 2, pSize ) ];

	// hermite polynomials
	const float h1 = 2.0f * u * u * u - 3.0f * u * u + 1.0f;
	const float h2 = -2.0f * u * u * u + 3.0f * u * u;
	const float h3 = u * u * u - 2.0f * u * u + u;
	const float h4 = u * u * u - u * u;

	// tangents
	const Vector3<float> t1 = ( p2 - p0 ) * m_Tightness;
	const Vector3<float> t2 = ( p3 - p1 ) * m_Tightness;

	// interpolated point
	node->SetPosition(p1 * h1 + p2 * h2 + t1 * h3 + t2 * h4);
}


SceneNodeAnimator* SceneNodeAnimatorFollowSpline::CreateClone(SceneNode* node)
{
	SceneNodeAnimatorFollowSpline * newAnimator =
		new SceneNodeAnimatorFollowSpline(m_StartTime, m_Points, m_Speed, m_Tightness);

	return newAnimator;
}

