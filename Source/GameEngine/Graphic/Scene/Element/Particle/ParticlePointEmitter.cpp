// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticlePointEmitter.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

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
			Quaternion<float> tgt = Rotation<3, float>(
				AxisAngle<3, float>(mParticle.mVector, Randomizer::FRand() * mMaxAngleDegrees));
			mParticle.mVector = HProject(Rotate(tgt, Vector4<float> { 0.0f, 0.0f, 1.0f, 0.0f }));
			tgt = Rotation<3, float>(
				AxisAngle<3, float>(mParticle.mVector, Randomizer::FRand() * mMaxAngleDegrees));
			mParticle.mVector = HProject(Rotate(tgt, Vector4<float> { 1.0f, 0.0f, 0.0f, 0.0f }));
			tgt = Rotation<3, float>(
				AxisAngle<3, float>(mParticle.mVector, Randomizer::FRand() * mMaxAngleDegrees));
			mParticle.mVector = HProject(Rotate(tgt, Vector4<float> { 0.0f, 1.0f, 0.0f, 0.0f }));
		}

		mParticle.mEndTime = now + mMinLifeTime;
		if (mMaxLifeTime != mMinLifeTime)
			mParticle.mEndTime += Randomizer::Rand() % (mMaxLifeTime - mMinLifeTime);

		if (mMinStartColor==mMaxStartColor)
			mParticle.mColor=mMinStartColor;
		else
			mParticle.mColor = Function<float>::Lerp(mMinStartColor, mMaxStartColor, Randomizer::FRand());

		mParticle.mStartColor = mParticle.mColor;
		mParticle.mStartVector = mParticle.mVector;

		if (mMinStartSize==mMaxStartSize)
			mParticle.mStartSize = mMinStartSize;
		else
			mParticle.mStartSize = Function<float>::Lerp(mMinStartSize, mMaxStartSize, Randomizer::FRand());
		mParticle.mSize = mParticle.mStartSize;

		outArray = &mParticle;
		return 1;
	}

	return 0;
}