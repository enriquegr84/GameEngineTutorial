// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticlePointEmitter.h"

#include "Core/OS/os.h"

//! constructor
ParticlePointEmitter::ParticlePointEmitter(
	const Vector3<float>& direction, 
	unsigned int minParticlesPerSecond, unsigned int maxParticlesPerSecond, 
	eastl::array<float, 4> minStartColor, eastl::array<float, 4> maxStartColor, 
	unsigned int lifeTimeMin, unsigned int lifeTimeMax, int maxAngleDegrees,
	const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize)
:	mDirection(direction), mMinStartSize(minStartSize), 
	mMaxStartSize(maxStartSize),
	mMinParticlesPerSecond(minParticlesPerSecond),
	mMaxParticlesPerSecond(maxParticlesPerSecond),
	mMinStartColor(minStartColor), mMaxStartColor(maxStartColor),
	mMinLifeTime(lifeTimeMin), mMaxLifeTime(lifeTimeMax),
	mMaxAngleDegrees(maxAngleDegrees), mTime(0), mEmitted(0)
{
	#ifdef _DEBUG
	//setDebugName("ParticlePointEmitter");
	#endif
}

//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
int ParticlePointEmitter::Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray)
{
	mTime += timeSinceLastCall;

	const unsigned int pps = (mMaxParticlesPerSecond - mMinParticlesPerSecond);
	const float perSecond = pps ? ((float)mMinParticlesPerSecond + Randomizer::FRand() * pps) : mMinParticlesPerSecond;
	const float everyWhatMillisecond = 1000.0f / perSecond;

	if (mTime > everyWhatMillisecond)
	{
		mTime = 0;
		mParticle.mStartTime = now;
		mParticle.mVector = mDirection;

		if (mMaxAngleDegrees)
		{
			Vector3<float> tgt = mDirection;
			tgt.RotateXYBy(Randomizer::FRand() * mMaxAngleDegrees);
			tgt.RotateYZBy(Randomizer::FRand() * mMaxAngleDegrees);
			tgt.RotateXZBy(Randomizer::FRand() * mMaxAngleDegrees);
			mParticle.mVector = tgt;
		}

		mParticle.mEndTime = now + mMinLifeTime;
		if (mMaxLifeTime != mMinLifeTime)
			mParticle.mEndTime += Randomizer::Rand() % (mMaxLifeTime - mMinLifeTime);

		if (mMinStartColor==mMaxStartColor)
			mParticle.mColor=mMinStartColor;
		else
			mParticle.mColor = mMinStartColor.GetInterpolated(mMaxStartColor, Randomizer::FRand());

		mParticle.mStartColor = mParticle.mColor;
		mParticle.mStartVector = mParticle.mVector;

		if (mMinStartSize==mMaxStartSize)
			mParticle.mStartSize = mMinStartSize;
		else
			mParticle.mStartSize = mMinStartSize.GetInterpolated(mMaxStartSize, Randomizer::FRand());
		mParticle.mSize = mParticle.mStartSize;

		outArray = &mParticle;
		return 1;
	}

	return 0;
}