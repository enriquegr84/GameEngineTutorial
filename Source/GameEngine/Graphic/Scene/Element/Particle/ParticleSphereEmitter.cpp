// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleSphereEmitter.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

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
			particle.mPos = Vector3<float>{ mCenter[0] + distance, mCenter[1], mCenter[2] + distance };
			Quaternion<float> tgt = Rotation<3, float>(
				AxisAngle<3, float>(particle.mPos, Randomizer::FRand() * 360));
			particle.mPos = HProject(Rotate(tgt, Vector4<float> { 0.0f, 0.0f, 1.0f, 0.0f }));
			tgt = Rotation<3, float>(
				AxisAngle<3, float>(particle.mPos, Randomizer::FRand() * 360));
			particle.mPos = HProject(Rotate(tgt, Vector4<float> { 1.0f, 0.0f, 0.0f, 0.0f }));
			tgt = Rotation<3, float>(
				AxisAngle<3, float>(particle.mPos, Randomizer::FRand() * 360));
			particle.mPos = HProject(Rotate(tgt, Vector4<float> { 0.0f, 1.0f, 0.0f, 0.0f }));
			//particle.mPos.RotateXYBy(Randomizer::FRand() * 360.f, mCenter );
			//particle.mPos.RotateYZBy(Randomizer::FRand() * 360.f, mCenter );
			//particle.mPos.RotateXZBy(Randomizer::FRand() * 360.f, mCenter );

			particle.mStartTime = now;
			particle.mVector = mDirection;

			if(mMaxAngleDegrees)
			{
				particle.mVector = mDirection;

				Quaternion<float> tgt = Rotation<3, float>(
					AxisAngle<3, float>(particle.mVector, Randomizer::FRand() * mMaxAngleDegrees));
				particle.mVector = HProject(Rotate(tgt, Vector4<float> { 0.0f, 0.0f, 1.0f, 0.0f }));
				tgt = Rotation<3, float>(
					AxisAngle<3, float>(particle.mVector, Randomizer::FRand() * mMaxAngleDegrees));
				particle.mVector = HProject(Rotate(tgt, Vector4<float> { 1.0f, 0.0f, 0.0f, 0.0f }));
				tgt = Rotation<3, float>(
					AxisAngle<3, float>(particle.mVector, Randomizer::FRand() * mMaxAngleDegrees));
				particle.mVector = HProject(Rotate(tgt, Vector4<float> { 0.0f, 1.0f, 0.0f, 0.0f }));
			}

			particle.mEndTime = now + mMinLifeTime;
			if (mMaxLifeTime != mMinLifeTime)
				particle.mEndTime += Randomizer::Rand() % (mMaxLifeTime - mMinLifeTime);

			if (mMinStartColor==mMaxStartColor)
				particle.mColor=mMinStartColor;
			else
				particle.mColor = Function<float>::Lerp(mMinStartColor, mMaxStartColor, Randomizer::FRand());

			particle.mStartColor = particle.mColor;
			particle.mStartVector = particle.mVector;

			if (mMinStartSize==mMaxStartSize)
				particle.mStartSize = mMinStartSize;
			else
				particle.mStartSize = Function<float>::Lerp(mMinStartSize, mMaxStartSize, Randomizer::FRand());
			particle.mSize = particle.mStartSize;

			mParticles.push_back(particle);
		}

		outArray = mParticles.data();

		return mParticles.size();
	}

	return 0;
}