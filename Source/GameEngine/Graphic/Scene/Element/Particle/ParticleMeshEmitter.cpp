// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "GameEngineStd.h"

#include "ParticleMeshEmitter.h"

//! constructor
ParticleMeshEmitter::ParticleMeshEmitter(
	const eastl::shared_ptr<Mesh>& mesh, bool useNormalDirection,
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
	mMinParticlesPerSecond(minParticlesPerSecond), mMaxParticlesPerSecond(maxParticlesPerSecond),
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
			if( mEveryMeshVertex )
			{
				for( unsigned int j=0; j<mParticleMesh->GetMeshBufferCount(); ++j )
				{
					for( unsigned int k=0; k<ParticleMesh->GetMeshBuffer(j)->GetVertexCount(); ++k )
					{
						p.pos = ParticleMesh->GetMeshBuffer(j)->GetPosition(k);
						if( mUseNormalDirection )
							p.vector = mParticleMesh->GetMeshBuffer(j)->GetNormal(k) / mNormalDirectionModifier;
						else
							p.vector = mDirection;

						p.startTime = now;

						if( mMaxAngleDegrees )
						{
							Vector3<float> tgt = p.vector;
							tgt.RotateXYBy(Randomizer::frand() * mMaxAngleDegrees);
							tgt.RotateYZBy(Randomizer::frand() * mMaxAngleDegrees);
							tgt.RotateXZBy(Randomizer::frand() * mMaxAngleDegrees);
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
				}
			}
			else
			{
				const int randomMB = (mMBNumber < 0) ? (Randomizer::rand() % mMBCount) : mMBNumber;

				unsigned int vertexNumber = mParticleMesh->GetMeshBuffer(randomMB)->GetVertexCount();
				if (!vertexNumber)
					continue;
				vertexNumber = Randomizer::rand() % vertexNumber;

				p.pos = mParticleMesh->GetMeshBuffer(randomMB)->GetPosition(vertexNumber);
				if( mUseNormalDirection )
					p.vector = mParticleMesh->GetMeshBuffer(randomMB)->GetNormal(vertexNumber) /mNormalDirectionModifier;
				else
					p.vector = mDirection;

				p.startTime = now;

				if( mMaxAngleDegrees )
				{
					Vector3<float> tgt = Direction;
					tgt.RotateXYBy(Randomizer::frand() * mMaxAngleDegrees);
					tgt.RotateYZBy(Randomizer::frand() * mMaxAngleDegrees);
					tgt.RotateXZBy(Randomizer::frand() * mMaxAngleDegrees);
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
		}

		outArray = mParticles.data();

		return mParticles.size();
	}

	return 0;
}


//! Set Mesh to emit particles from
void ParticleMeshEmitter::SetMesh(const eastl::shared_ptr<Mesh>& mesh)
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