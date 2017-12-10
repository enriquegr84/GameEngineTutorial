// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleRingEmitter.h"

//! constructor
ParticleRingEmitter::ParticleRingEmitter(
	const Vector3<float>& center, float radius, float ringThickness, const Vector3<float>& direction, 
	unsigned int minParticlesPerSecond, unsigned int maxParticlesPerSecond, const Color& minStartColor,
	const Color& maxStartColor, unsigned int lifeTimeMin, unsigned int lifeTimeMax, int maxAngleDegrees, 
	const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize )
:	m_Center(center), m_Radius(radius), m_RingThickness(ringThickness), m_Direction(direction),
	m_MaxStartSize(maxStartSize), m_MinStartSize(minStartSize), m_MinParticlesPerSecond(minParticlesPerSecond),
	m_MaxParticlesPerSecond(maxParticlesPerSecond), m_MinStartColor(minStartColor), m_MaxStartColor(maxStartColor),
	m_MinLifeTime(lifeTimeMin), m_MaxLifeTime(lifeTimeMax), m_Time(0), m_Emitted(0), m_MaxAngleDegrees(maxAngleDegrees)
{
	#ifdef _DEBUG
	//setDebugName("ParticleRingEmitter");
	#endif
}


//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
int ParticleRingEmitter::Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray)
{
	mTime += timeSinceLastCall;

	unsigned int pps = (mMaxParticlesPerSecond - mMinParticlesPerSecond);
	float perSecond = pps ? ((float)mMinParticlesPerSecond + Randomizer::frand() * pps) : mMinParticlesPerSecond;
	float everyWhatMillisecond = 1000.0f / perSecond;

	if(mTime > everyWhatMillisecond)
	{
		mParticles.clear();
		unsigned int amount = (unsigned int)((mTime / everyWhatMillisecond) + 0.5f);
		mTime = 0;
		Particle p;

		if(amount > mMaxParticlesPerSecond*2)
			amount = mMaxParticlesPerSecond * 2;

		for(unsigned int i=0; i<amount; ++i)
		{
			float distance = Randomizer::frand() * mRingThickness * 0.5f;
			if (Randomizer::rand() % 2)
				distance -= mRadius;
			else
				distance += mRadius;

			p.pos.set(mCenter.X + distance, mCenter.Y, mCenter.Z + distance);
			p.pos.RotateXZBy(Randomizer::frand() * 360, mCenter );

			p.startTime = now;
			p.vector = mDirection;

			if(mMaxAngleDegrees)
			{
				Vector3<float> tgt = mDirection;
				tgt.RotateXYBy(Randomizer::frand() * mMaxAngleDegrees, mCenter );
				tgt.RotateYZBy(Randomizer::frand() * mMaxAngleDegrees, mCenter );
				tgt.RotateXZBy(Randomizer::frand() * mMaxAngleDegrees, mCenter );
				p.vector = tgt;
			}

			p.endTime = now + mMinLifeTime;
			if (mMaxLifeTime != mMinLifeTime)
				p.endTime += Randomizer::rand() % (mMaxLifeTime - mMinLifeTime);

			if (mMinStartColor==mMaxStartColor)
				p.color=mMinStartColor;
			else
				p.color = mMinStartColor.GetInterpolated(mMaxStartColor, Randomizer::frand());

			p.startColor = p.color;
			p.startVector = p.vector;

			if (mMinStartSize==mMaxStartSize)
				p.startSize = mMinStartSize;
			else
				p.startSize = mMinStartSize.GetInterpolated(mMaxStartSize, Randomizer::frand());
			p.size = p.startSize;

			mParticles.push_back(p);
		}

		outArray = mParticles.data();

		return mParticles.size();
	}

	return 0;
}
