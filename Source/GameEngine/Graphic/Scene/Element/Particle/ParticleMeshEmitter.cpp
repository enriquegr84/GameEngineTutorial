// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "GameEngineStd.h"

#include "ParticleMeshEmitter.h"

#include "Core/OS/os.h"

//! constructor
ParticleMeshEmitter::ParticleMeshEmitter(
	const eastl::shared_ptr<BaseMesh>& mesh, bool useNormalDirection,
	const Vector3<float>& direction, float normalDirectionModifier, 
	int mbNumber, bool everyMeshVertex,
	unsigned int minParticlesPerSecond, unsigned int maxParticlesPerSecond,
	const eastl::array<float, 4>& minStartColor, 
	const eastl::array<float, 4>& maxStartColor,
	unsigned int lifeTimeMin, unsigned int lifeTimeMax, int maxAngleDegrees,
	const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize )
	: mParticleMesh(0), mTotalVertices(0), mMBCount(0), mMBNumber(mbNumber),
	mNormalDirectionModifier(normalDirectionModifier), mDirection(direction),
	mMaxStartSize(maxStartSize), mMinStartSize(minStartSize),
	mMinParticlesPerSecond(minParticlesPerSecond), 
	mMaxParticlesPerSecond(maxParticlesPerSecond),
	mMinStartColor(minStartColor), mMaxStartColor(maxStartColor),
	mMinLifeTime(lifeTimeMin), mMaxLifeTime(lifeTimeMax),
	mTime(0), mEmitted(0), mMaxAngleDegrees(maxAngleDegrees),
	mEveryMeshVertex(everyMeshVertex), mUseNormalDirection(useNormalDirection)
{
	#ifdef _DEBUG
	//setDebugName("ParticleMeshEmitter");
	#endif
	SetMesh(mesh);
}


//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
int ParticleMeshEmitter::Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray)
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
		if(amount > mMaxParticlesPerSecond * 2)
			amount = mMaxParticlesPerSecond * 2;

		for(unsigned int i=0; i<amount; ++i)
		{
			if( mEveryMeshVertex )
			{
				for( unsigned int j=0; j<mParticleMesh->GetMeshBufferCount(); ++j )
				{
					for( unsigned int k=0; k<mParticleMesh->GetMeshBuffer(j)->GetVertexCount(); ++k )
					{
						particle.mPos = mParticleMesh->GetMeshBuffer(j)->GetPosition(k);
						if( mUseNormalDirection )
							particle.mVector = mParticleMesh->GetMeshBuffer(j)->GetNormal(k) / mNormalDirectionModifier;
						else
							particle.mVector = mDirection;

						particle.mStartTime = now;

						if( mMaxAngleDegrees )
						{
							Vector3<float> tgt = particle.mVector;
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
						particle.mStartColor = particle.mVector;

						if (mMinStartSize==mMaxStartSize)
							particle.mStartSize = mMinStartSize;
						else
							particle.mStartSize = mMinStartSize.GetInterpolated(mMaxStartSize, Randomizer::FRand());
						particle.mSize = particle.mStartSize;

						mParticles.push_back(particle);
					}
				}
			}
			else
			{
				const int randomMB = (mMBNumber < 0) ? (Randomizer::Rand() % mMBCount) : mMBNumber;

				unsigned int vertexNumber = mParticleMesh->GetMeshBuffer(randomMB)->GetVertexCount();
				if (!vertexNumber)
					continue;
				vertexNumber = Randomizer::Rand() % vertexNumber;

				particle.mPos = mParticleMesh->GetMeshBuffer(randomMB)->GetPosition(vertexNumber);
				if( mUseNormalDirection )
					particle.mVector = mParticleMesh->GetMeshBuffer(randomMB)->GetNormal(vertexNumber) /mNormalDirectionModifier;
				else
					particle.mVector = mDirection;

				particle.mStartTime = now;

				if( mMaxAngleDegrees )
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
		}

		outArray = mParticles.data();

		return mParticles.size();
	}

	return 0;
}


//! Set Mesh to emit particles from
void ParticleMeshEmitter::SetMesh(const eastl::shared_ptr<BaseMesh>& mesh)
{
	mParticleMesh = mesh;

	mTotalVertices = 0;
	mMBCount = 0;
	mVertexPerMeshBufferList.clear();

	if ( !mParticleMesh )
		return;

	mMBCount = mParticleMesh->GetMeshBufferCount();
	for( unsigned int i = 0; i < mMBCount; ++i )
	{
		mVertexPerMeshBufferList.push_back( mParticleMesh->GetMeshBuffer(i)->GetVertexCount() );
		mTotalVertices += mParticleMesh->GetMeshBuffer(i)->GetVertexCount();
	}
}