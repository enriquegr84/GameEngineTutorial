// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SceneNodeAnimatorFlyCircle.h"

#include "GameEngine/GameEngine.h"
#include "OS/os.h"

//! constructor
SceneNodeAnimatorFlyCircle::SceneNodeAnimatorFlyCircle(unsigned int time, const Vector3<float>& center, 
	float radius, float speed, const Vector3<float>& direction, float radiusEllipsoid)
:	m_Center(center), m_Direction(direction),  m_Radius(radius), 
	m_RadiusEllipsoid(radiusEllipsoid), m_Speed(speed), m_StartTime(time)
{
	#ifdef _DEBUG
	//setDebugName("SceneNodeAnimatorFlyCircle");
	#endif
	Init();
}


void SceneNodeAnimatorFlyCircle::Init()
{
	m_Direction.Normalize();

	if (m_Direction.Y != 0)
		m_VecV = Vector3<float>(50,0,0).CrossProduct(m_Direction).Normalize();
	else
		m_VecV = Vector3<float>(0,50,0).CrossProduct(m_Direction).Normalize();
	m_VecU = m_VecV.CrossProduct(m_Direction).Normalize();
}


//! animates a scene node
void SceneNodeAnimatorFlyCircle::AnimateNode(SceneNode* node, unsigned int timeMs)
{
	if ( 0 == node )
		return;

	float time;

	// Check for the condition where the StartTime is in the future.
	if(m_StartTime > timeMs)
		time = ((int)timeMs - (int)m_StartTime) * m_Speed;
	else
		time = (timeMs-m_StartTime) * m_Speed;

//	node->SetPosition(m_Center + Radius * ((m_VecU*cosf(time)) + (m_VecV*sinf(time))));
	float r2 = m_RadiusEllipsoid == 0.f ? m_Radius : m_RadiusEllipsoid;
	node->SetPosition(m_Center + (m_Radius*cosf(time)*m_VecU) + (r2*sinf(time)*m_VecV ) );
}


SceneNodeAnimator* SceneNodeAnimatorFlyCircle::CreateClone(SceneNode* node)
{
	SceneNodeAnimatorFlyCircle* newAnimator =
		new SceneNodeAnimatorFlyCircle(m_StartTime, m_Center, m_Radius, m_Speed, m_Direction, m_RadiusEllipsoid);

	return newAnimator;
}