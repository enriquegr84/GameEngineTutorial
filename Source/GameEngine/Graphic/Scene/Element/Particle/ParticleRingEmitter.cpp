// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleRingEmitter.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

//! constructor
ParticleRingEmitter::ParticleRingEmitter(
	const Vector3<float>& center, float radius, float ringThickness, const Vector3<float>& direction, 
	unsigned int minParticlesPerSecond, unsigned int maxParticlesPerSecond, const SColor& minStartColor,
	const SColor& maxStartColor, unsigned int lifeTimeMin, unsigned int lifeTimeMax, int maxAngleDegrees,
	const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize )
:	mCenter(center), mRadius(radius), mRingThickness(ringThickness), mDirection(direction),
	mMaxStartSize(maxStartSize), mMinStartSize(minStartSize), mMinParticlesPerSecond(minParticlesPerSecond),
	mMaxParticlesPerSecond(maxParticlesPerSecond), mMinStartColor(minStartColor), mMaxStartColor(maxStartColor),
	mMinLifeTime(lifeTimeMin), mMaxLifeTime(lifeTimeMax), mTime(0), mEmitted(0), mMaxAngleDegrees(maxAngleDegrees)
{

}


//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
int ParticleRingEmitter::Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray)
{
	mTime += timeSinceLastCall;

	unsigned int pps = (mMaxParticlesPerSecond - mMinParticlesPerSecond);
	float perSecond = pps ? ((float)mMinParticlesPerSecond + Randomizer::FRand() * pps) : mMinParticlesPerSecond;
	float everyWhatMillisecond = 1000.0f / perSecond;

	if(mTime > everyWhatMillisecond)
	{
		mParticles.clear();
		unsigned int amount = (unsigned int)((mTime / everyWhatMillisecond) + 0.5f);
		mTime = 0;
		Particle particle;

		if(amount > mMaxParticlesPerSecond*2)
			amount = mMaxParticlesPerSecond * 2;

		for(unsigned int i=0; i<amount; ++i)
		{
			float distance = Randomizer::FRand() * mRingThickness * 0.5f;
			if (Randomizer::Rand() % 2)
				distance -= mRadius;
			else
				distance += mRadius;

			particle.mPos = Vector3<float>{ mCenter[0] + distance, mCenter[1], mCenter[2] + distance };
			Quaternion<float> tgt = Rotation<3, float>(
				AxisAngle<3, float>(particle.mPos, Randomizer::FRand() * 360));
			particle.mPos = HProject(Rotate(tgt, Vector4<float>::Unit(2)));
			//p.mPos.RotateXZBy(Randomizer::FRand() * 360, mCenter);

			particle.mStartTime = now;
			particle.mVector = mDirection;

			if(mMaxAngleDegrees)
			{
				particle.mVector = mDirection;

				Quaternion<float> tgt = Rotation<3, float>(
					AxisAngle<3, float>(particle.mVector, Randomizer::FRand() * mMaxAngleDegrees * (float)GE_C_DEG_TO_RAD));
				particle.mVector = HProject(Rotate(tgt, Vector4<float>::Unit(1)));
				tgt = Rotation<3, float>(
					AxisAngle<3, float>(particle.mVector, Randomizer::FRand() * mMaxAngleDegrees * (float)GE_C_DEG_TO_RAD));
				particle.mVector = HProject(Rotate(tgt, Vector4<float>::Unit(0)));
				tgt = Rotation<3, float>(
					AxisAngle<3, float>(particle.mVector, Randomizer::FRand() * mMaxAngleDegrees * (float)GE_C_DEG_TO_RAD));
				particle.mVector = HProject(Rotate(tgt, Vector4<float>::Unit(2)));
			}

			particle.mEndTime = now + mMinLifeTime;
			if (mMaxLifeTime != mMinLifeTime)
				particle.mEndTime += Randomizer::Rand() % (mMaxLifeTime - mMinLifeTime);

			if (mMinStartColor==mMaxStartColor)
				particle.mColor=mMinStartColor;
			else
				particle.mColor = SColorF(
                    Function<float>::Lerp(mMinStartColor.ToArray(), mMaxStartColor.ToArray(), Randomizer::FRand()));

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
