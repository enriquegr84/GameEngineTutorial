// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleSphereEmitter.h"

#include "OS/os.h"

//! constructor
ParticleSphereEmitter::ParticleSphereEmitter(
	const Vector3<float>& center, float radius,
	const Vector3<float>& direction, unsigned int minParticlesPerSecond,
	unsigned int maxParticlesPerSecond, const Color& minStartColor,
	const Color& maxStartColor, unsigned int lifeTimeMin, unsigned int lifeTimeMax,
	int maxAngleDegrees,
	const Vector2<float>& minStartSize,
	const Vector2<float>& maxStartSize )
:	m_Center(center), m_Radius(radius), m_Direction(direction),
	m_MinStartSize(minStartSize), m_MaxStartSize(maxStartSize),
	m_MinParticlesPerSecond(minParticlesPerSecond),
	m_MaxParticlesPerSecond(maxParticlesPerSecond),
	m_MinStartColor(minStartColor), m_MaxStartColor(maxStartColor),
	m_MinLifeTime(lifeTimeMin), m_MaxLifeTime(lifeTimeMax),
	m_Time(0), m_Emitted(0), m_MaxAngleDegrees(maxAngleDegrees)
{
	#ifdef _DEBUG
	//setDebugName("CParticleSphereEmitter");
	#endif
}

//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
int ParticleSphereEmitter::Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray)
{
	m_Time += timeSinceLastCall;

	const unsigned int pps = (m_MaxParticlesPerSecond - m_MinParticlesPerSecond);
	const float perSecond = pps ? ((float)m_MinParticlesPerSecond + Randomizer::frand() * pps) : m_MinParticlesPerSecond;
	const float everyWhatMillisecond = 1000.0f / perSecond;

	if(m_Time > everyWhatMillisecond)
	{
		unsigned int amount = (unsigned int)((m_Time / everyWhatMillisecond) + 0.5f);
		m_Time = 0;
		Particle p;

		if(amount > m_MaxParticlesPerSecond*2)
			amount = m_MaxParticlesPerSecond * 2;

		for(unsigned int i=0; i<amount; ++i)
		{
			// Random distance from center
			const float distance = Randomizer::frand() * m_Radius;

			// Random direction from center
			p.pos.set(m_Center + distance);
			p.pos.RotateXYBy(Randomizer::frand() * 360.f, m_Center );
			p.pos.RotateYZBy(Randomizer::frand() * 360.f, m_Center );
			p.pos.RotateXZBy(Randomizer::frand() * 360.f, m_Center );

			p.startTime = now;
			p.vector = m_Direction;

			if(m_MaxAngleDegrees)
			{
				Vector3<float> tgt = m_Direction;
				tgt.RotateXYBy(Randomizer::frand() * m_MaxAngleDegrees);
				tgt.RotateYZBy(Randomizer::frand() * m_MaxAngleDegrees);
				tgt.RotateXZBy(Randomizer::frand() * m_MaxAngleDegrees);
				p.vector = tgt;
			}

			p.endTime = now + m_MinLifeTime;
			if (m_MaxLifeTime != m_MinLifeTime)
				p.endTime += Randomizer::rand() % (m_MaxLifeTime - m_MinLifeTime);

			if (m_MinStartColor==m_MaxStartColor)
				p.color=m_MinStartColor;
			else
				p.color = m_MinStartColor.GetInterpolated(m_MaxStartColor, Randomizer::frand());

			p.startColor = p.color;
			p.startVector = p.vector;

			if (m_MinStartSize==m_MaxStartSize)
				p.startSize = m_MinStartSize;
			else
				p.startSize = m_MinStartSize.GetInterpolated(m_MaxStartSize, Randomizer::frand());
			p.size = p.startSize;

			m_Particles.push_back(p);
		}

		outArray = m_Particles.data();

		return m_Particles.size();
	}

	return 0;
}