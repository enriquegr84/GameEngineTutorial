// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleCylinderEmitter.h"

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
	const float perSecond = pps ? ((float)mMinParticlesPerSecond + Randomizer::frand() * pps) : mMinParticlesPerSecond;
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
			const float distance = (!mOutlineOnly) ? (Randomizer::frand() * mRadius) : mRadius;

			// Random direction from center
			p.pos.set(mCenter.X + distance, mCenter.Y, mCenter.Z + distance);
			p.pos.RotateXZBy(Randomizer::frand() * 360, mCenter);

			// Random length
			const float length = Randomizer::frand() * mLength;

			// Random point along the cylinders length
			p.pos += mNormal * length;

			p.startTime = now;
			p.vector = mDirection;

			if ( mMaxAngleDegrees )
			{
				Vector3<float> tgt = mDirection;
				tgt.RotateXYBy(Randomizer::frand() * mMaxAngleDegrees);
				tgt.RotateYZBy(Randomizer::frand() * mMaxAngleDegrees);
				tgt.RotateXZBy(Randomizer::frand() * mMaxAngleDegrees);
				p.vector = tgt;
			}

			p.endTime = now + mMinLifeTime;
			if (mMaxLifeTime != mMinLifeTime)
				p.endTime += Randomizer::rand() % (mMaxLifeTime - mMinLifeTime);

			if (mMinStartColor==mMaxStartColor)
				p.color = mMinStartColor;
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
