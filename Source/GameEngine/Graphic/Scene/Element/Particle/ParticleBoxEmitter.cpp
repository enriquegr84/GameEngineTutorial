// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleBoxEmitter.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

//! constructor
ParticleBoxEmitter::ParticleBoxEmitter( 
	const AlignedBox3<float>& box, const Vector3<float>& direction,
	unsigned int minParticlesPerSecond, unsigned int maxParticlesPerSecond, 
	eastl::array<float, 4> minStartColor, eastl::array<float, 4> maxStartColor,
	unsigned int lifeTimeMin, unsigned int lifeTimeMax, int maxAngleDegrees, 
	const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize)
:	mBox(box), mDirection(direction), mMaxStartSize(maxStartSize), mMinStartSize(minStartSize),
	mMinParticlesPerSecond(minParticlesPerSecond), mMaxParticlesPerSecond(maxParticlesPerSecond),
	mMinStartColor(minStartColor), mMaxStartColor(maxStartColor), mMinLifeTime(lifeTimeMin), 
	mMaxLifeTime(lifeTimeMax), mTime(0), mEmitted(0), mMaxAngleDegrees(maxAngleDegrees)
{

}


//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
int ParticleBoxEmitter::Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray)
{
	mTime += timeSinceLastCall;

	const unsigned int pps = (mMaxParticlesPerSecond - mMinParticlesPerSecond);
	const float perSecond = pps ? ((float)mMinParticlesPerSecond + Randomizer::FRand() * pps) : mMinParticlesPerSecond;
	const float everyWhatMillisecond = 1000.0f / perSecond;

	if (mTime > everyWhatMillisecond)
	{
		mParticles.clear();
		unsigned int amount = (unsigned int)((mTime / everyWhatMillisecond) + 0.5f);
		mTime = 0;
		Particle particle;
		Vector3<float> extent = mBox.mMax - mBox.mMin;

		if (amount > mMaxParticlesPerSecond*2)
			amount = mMaxParticlesPerSecond * 2;

		for (unsigned int i=0; i<amount; ++i)
		{
			particle.mPos[0] = mBox.mMin[0] + Randomizer::FRand() * extent[0];
			particle.mPos[1] = mBox.mMin[1] + Randomizer::FRand() * extent[1];
			particle.mPos[2] = mBox.mMin[2] + Randomizer::FRand() * extent[2];

			particle.mStartTime = now;
			particle.mVector = mDirection;

			if (mMaxAngleDegrees)
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

