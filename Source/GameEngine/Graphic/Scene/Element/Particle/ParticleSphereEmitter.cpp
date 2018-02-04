// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleSphereEmitter.h"

#include "Core/OS/os.h"

//! constructor
ParticleSphereEmitter::ParticleSphereEmitter(
	const Vector3<float>& center, float radius,
	const Vector3<float>& direction, unsigned int minParticlesPerSecond,
	unsigned int maxParticlesPerSecond, const eastl::array<float, 4>& minStartColor,
	const eastl::array<float, 4>& maxStartColor, unsigned int lifeTimeMin, unsigned int lifeTimeMax,
	int maxAngleDegrees, const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize )
:	mCenter(center), mRadius(radius), mDirection(direction),
	mMinStartSize(minStartSize), mMaxStartSize(maxStartSize),
	mMinParticlesPerSecond(minParticlesPerSecond),
	mMaxParticlesPerSecond(maxParticlesPerSecond),
	mMinStartColor(minStartColor), mMaxStartColor(maxStartColor),
	mMinLifeTime(lifeTimeMin), mMaxLifeTime(lifeTimeMax),
	mTime(0), mEmitted(0), mMaxAngleDegrees(maxAngleDegrees)
{
	#ifdef _DEBUG
	//setDebugName("CParticleSphereEmitter");
	#endif
}

//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
int ParticleSphereEmitter::Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray)
{
	mTime += timeSinceLastCall;

	const unsigned int pps = (mMaxParticlesPerSecond - mMinParticlesPerSecond);
	const float perSecond = pps ? ((float)mMinParticlesPerSecond + Randomizer::FRand() * pps) : mMinParticlesPerSecond;
	const float everyWhatMillisecond = 1000.0f / perSecond;

	if(mTime > everyWhatMillisecond)
	{
		unsigned int amount = (unsigned int)((mTime / everyWhatMillisecond) + 0.5f);
		mTime = 0;
		Particle particle;

		if(amount > mMaxParticlesPerSecond*2)
			amount = mMaxParticlesPerSecond * 2;

		for(unsigned int i=0; i<amount; ++i)
		{
			// Random distance from center
			const float distance = Randomizer::FRand() * mRadius;

			// Random direction from center
			particle.mPos.set(mCenter + distance);
			particle.mPos.RotateXYBy(Randomizer::FRand() * 360.f, mCenter );
			particle.mPos.RotateYZBy(Randomizer::FRand() * 360.f, mCenter );
			particle.mPos.RotateXZBy(Randomizer::FRand() * 360.f, mCenter );

			particle.mStartTime = now;
			particle.mVector = mDirection;

			if(mMaxAngleDegrees)
			{
				Vector3<float> tgt = mDirection;
				tgt.RotateXYBy(Randomizer::FRand() * mMaxAngleDegrees);
				tgt.RotateYZBy(Randomizer::FRand() * mMaxAngleDegrees);
				tgt.RotateXZBy(Randomizer::FRand() * mMaxAngleDegrees);
				particle.mVector = tgt;
			}

			particle.mEndTime = now + mMinLifeTime;
			if (mMaxLifeTime != mMinLifeTime)
				particle.mEndTime += Randomizer::Rand() % (mMaxLifeTime - mMinLifeTime);

			if (mMinStartColor==mMaxStartColor)
				particle.mColor=mMinStartColor;
			else
				particle.mColor = mMinStartColor.GetInterpolated(mMaxStartColor, Randomizer::FRand());

			particle.mStartColor = particle.mColor;
			particle.mStartVector = particle.mVector;

			if (mMinStartSize==mMaxStartSize)
				particle.mStartSize = mMinStartSize;
			else
				particle.mStartSize = mMinStartSize.GetInterpolated(mMaxStartSize, Randomizer::FRand());
			particle.mSize = particle.mStartSize;

			mParticles.push_back(particle);
		}

		outArray = mParticles.data();

		return mParticles.size();
	}

	return 0;
}