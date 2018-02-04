// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleCylinderEmitter.h"

#include "Core/OS/os.h"

//! constructor
ParticleCylinderEmitter::ParticleCylinderEmitter(
	const Vector3<float>& center, float radius, 
	const Vector3<float>& normal, float length, 
	bool outlineOnly, const Vector3<float>& direction, 
	unsigned int minParticlesPerSecond, 
	unsigned int maxParticlesPerSecond,
	const eastl::array<float, 4>& minStartColor, 
	const eastl::array<float, 4>& maxStartColor,
	unsigned int lifeTimeMin, unsigned int lifeTimeMax, int maxAngleDegrees,
	const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize )
	: mCenter(center), mNormal(normal), mDirection(direction),
	mMaxStartSize(maxStartSize), mMinStartSize(minStartSize),
	mMinParticlesPerSecond(minParticlesPerSecond),
	mMaxParticlesPerSecond(maxParticlesPerSecond),
	mMinStartColor(minStartColor), mMaxStartColor(maxStartColor),
	mMinLifeTime(lifeTimeMin), mMaxLifeTime(lifeTimeMax),
	mRadius(radius), mLength(length), mTime(0), mEmitted(0),
	mMaxAngleDegrees(maxAngleDegrees), mOutlineOnly(outlineOnly)
{
	#ifdef _DEBUG
	//setDebugName("ParticleCylinderEmitter");
	#endif
}


//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
int ParticleCylinderEmitter::Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray)
{
	mTime += timeSinceLastCall;

	const unsigned int pps = (mMaxParticlesPerSecond - mMinParticlesPerSecond);
	const float perSecond = pps ? ((float)mMinParticlesPerSecond + Randomizer::FRand() * pps) : mMinParticlesPerSecond;
	const float everyWhatMillisecond = 1000.0f / perSecond;

	if(mTime > everyWhatMillisecond)
	{
		unsigned int amount = (unsigned int)((mTime / everyWhatMillisecond) + 0.5f);
		mTime = 0;
		Particle p;

		if(amount > mMaxParticlesPerSecond * 2)
			amount = mMaxParticlesPerSecond * 2;

		for(unsigned int i=0; i<amount; ++i)
		{
			// Random distance from center if outline only is not true
			const float distance = (!mOutlineOnly) ? (Randomizer::FRand() * mRadius) : mRadius;

			// Random direction from center
			p.mPos.set(mCenter.X + distance, mCenter.Y, mCenter.Z + distance);
			p.mPos.RotateXZBy(Randomizer::FRand() * 360, mCenter);

			// Random length
			const float length = Randomizer::FRand() * mLength;

			// Random point along the cylinders length
			p.mPos += mNormal * length;

			p.mStartTime = now;
			p.mVector = mDirection;

			if ( mMaxAngleDegrees )
			{
				Vector3<float> tgt = mDirection;
				tgt.RotateXYBy(Randomizer::FRand() * mMaxAngleDegrees);
				tgt.RotateYZBy(Randomizer::FRand() * mMaxAngleDegrees);
				tgt.RotateXZBy(Randomizer::FRand() * mMaxAngleDegrees);
				p.mVector = tgt;
			}

			p.mEndTime = now + mMinLifeTime;
			if (mMaxLifeTime != mMinLifeTime)
				p.mEndTime += Randomizer::Rand() % (mMaxLifeTime - mMinLifeTime);

			if (mMinStartColor==mMaxStartColor)
				p.mColor = mMinStartColor;
			else
				p.mColor = mMinStartColor.GetInterpolated(mMaxStartColor, Randomizer::FRand());

			p.mStartColor = p.mColor;
			p.mStartVector = p.mVector;

			if (mMinStartSize==mMaxStartSize)
				p.mStartSize = mMinStartSize;
			else
				p.mStartSize = mMinStartSize.GetInterpolated(mMaxStartSize, Randomizer::FRand());
			p.mSize = p.mStartSize;

			mParticles.push_back(p);
		}

		outArray = mParticles.data();

		return mParticles.size();
	}

	return 0;
}
