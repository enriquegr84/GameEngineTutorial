// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleMeshEmitter.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

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
					const eastl::shared_ptr<MeshBuffer>& meshBuffer = mParticleMesh->GetMeshBuffer(j);
					for( unsigned int k=0; k<meshBuffer->GetVertice()->GetNumElements(); ++k )
					{
						particle.mPos = meshBuffer->Position(k);
						if( mUseNormalDirection )
							particle.mVector = meshBuffer->Normal(k) / mNormalDirectionModifier;
						else
							particle.mVector = mDirection;

						particle.mStartTime = now;

						if( mMaxAngleDegrees )
						{
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
				}
			}
			else
			{
				const int randomMB = (mMBNumber < 0) ? (Randomizer::Rand() % mMBCount) : mMBNumber;

				const eastl::shared_ptr<MeshBuffer>& meshBuffer = mParticleMesh->GetMeshBuffer(randomMB);
				if (!meshBuffer->GetVertice()->GetNumElements())
					continue;
				
				unsigned int vertexNumber = Randomizer::Rand() % meshBuffer->GetVertice()->GetNumElements();

				particle.mPos = meshBuffer->Position(vertexNumber);
				if( mUseNormalDirection )
					particle.mVector = meshBuffer->Normal(vertexNumber) /mNormalDirectionModifier;
				else
					particle.mVector = mDirection;

				particle.mStartTime = now;

				if( mMaxAngleDegrees )
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
		const eastl::shared_ptr<MeshBuffer>& meshBuffer = mParticleMesh->GetMeshBuffer(i);
		mVertexPerMeshBufferList.push_back(meshBuffer->GetVertice()->GetNumElements() );
		mTotalVertices += meshBuffer->GetVertice()->GetNumElements();
	}
}