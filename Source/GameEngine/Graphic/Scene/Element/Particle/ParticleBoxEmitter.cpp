// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleBoxEmitter.h"

#include "OS/os.h"

//! constructor
ParticleBoxEmitter::ParticleBoxEmitter( const AABBox3f& box, const Vector3<float>& direction,
	unsigned int minParticlesPerSecond, unsigned int maxParticlesPerSecond, Color minStartColor, Color maxStartColor,
	unsigned int lifeTimeMin, unsigned int lifeTimeMax, int maxAngleDegrees, const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize)
:	m_Box(box), m_Direction(direction), m_MaxStartSize(maxStartSize), m_MinStartSize(minStartSize),
	m_MinParticlesPerSecond(minParticlesPerSecond), m_MaxParticlesPerSecond(maxParticlesPerSecond),
	m_MinStartColor(minStartColor), m_MaxStartColor(maxStartColor), m_MinLifeTime(lifeTimeMin), 
	m_MaxLifeTime(lifeTimeMax), m_Time(0), m_Emitted(0), m_MaxAngleDegrees(maxAngleDegrees)
{
	#ifdef _DEBUG
	//setDebugName("CParticleBoxEmitter");
	#endif
}


//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
int ParticleBoxEmitter::Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray)
{
	m_Time += timeSinceLastCall;

	const unsigned int pps = (m_MaxParticlesPerSecond - m_MinParticlesPerSecond);
	const float perSecond = pps ? ((float)m_MinParticlesPerSecond + Randomizer::frand() * pps) : m_MinParticlesPerSecond;
	const float everyWhatMillisecond = 1000.0f / perSecond;

	if (m_Time > everyWhatMillisecond)
	{
		m_Particles.clear();
		unsigned int amount = (unsigned int)((m_Time / everyWhatMillisecond) + 0.5f);
		m_Time = 0;
		Particle p;
		const Vector3<float>& extent = m_Box.GetExtent();

		if (amount > m_MaxParticlesPerSecond*2)
			amount = m_MaxParticlesPerSecond * 2;

		for (unsigned int i=0; i<amount; ++i)
		{
			p.pos.X = m_Box.MinEdge.X + Randomizer::frand() * extent.X;
			p.pos.Y = m_Box.MinEdge.Y + Randomizer::frand() * extent.Y;
			p.pos.Z = m_Box.MinEdge.Z + Randomizer::frand() * extent.Z;

			p.startTime = now;
			p.vector = m_Direction;

			if (m_MaxAngleDegrees)
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

