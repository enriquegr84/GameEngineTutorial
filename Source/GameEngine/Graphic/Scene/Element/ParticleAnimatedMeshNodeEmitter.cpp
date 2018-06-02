// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleAnimatedMeshNodeEmitter.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

//! constructor
ParticleAnimatedMeshNodeEmitter::ParticleAnimatedMeshNodeEmitter(const ActorId actorId, 
		PVWUpdater& updater, WeakBaseRenderComponentPtr renderComponent, 
		const eastl::shared_ptr<AnimatedMeshNode>& node, bool useNormalDirection, 
		const Vector3<float>& direction, float normalDirectionModifier, int mbNumber, 
		bool everyMeshVertex, unsigned int minParticlesPerSecond, unsigned int maxParticlesPerSecond,
		const eastl::array<float, 4>& minStartColor, const eastl::array<float, 4>& maxStartColor, 
		unsigned int lifeTimeMin, unsigned int lifeTimeMax, int maxAngleDegrees, 
		const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize)
:	Node(actorId, renderComponent, RP_TRANSPARENT, NT_ANIMATED_MESH), mAnimatedNode(0), 
	mAnimatedMesh(0), mBaseMesh(0), mTotalVertices(0), mMBCount(0), mMBNumber(mbNumber), 
	mDirection(direction), mNormalDirectionModifier(normalDirectionModifier),
	mMinParticlesPerSecond(minParticlesPerSecond), mMaxParticlesPerSecond(maxParticlesPerSecond),
	mMinStartColor(minStartColor), mMaxStartColor(maxStartColor), mMinLifeTime(lifeTimeMin),
	mMaxLifeTime(lifeTimeMax), mMaxStartSize(maxStartSize), mMinStartSize(minStartSize), 
	mTime(0), mEmitted(0), mMaxAngleDegrees(maxAngleDegrees),
	mEveryMeshVertex(everyMeshVertex), mUseNormalDirection(useNormalDirection)
{
	#ifdef _DEBUG
	//setDebugName("ParticleAnimatedMeshNodeEmitter");
	#endif
	mPVWUpdater = updater;
	SetAnimatedMeshNode(node);
}


//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
int ParticleAnimatedMeshNodeEmitter::Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray)
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

		// Get Mesh for this frame
		const eastl::shared_ptr<BaseMesh>& frameMesh = 
			mAnimatedMesh->GetMesh((int)Function<float>::Floor(mAnimatedNode->GetFrameNr()), 
			255, mAnimatedNode->GetStartFrame(), mAnimatedNode->GetEndFrame() );

		for(unsigned int i=0; i<amount; ++i)
		{
			if( mEveryMeshVertex )
			{
				for(unsigned int j=0; j<frameMesh->GetMeshBufferCount(); ++j )
				{
					const MeshDescription& md = frameMesh->GetMeshBuffer(j)->mMesh->GetDescription();
					for(unsigned int k=0; k<md.mNumVertices; ++k )
					{
						particle.mPos = frameMesh->GetMeshBuffer(j)->mMesh->Position(k);
						if( mUseNormalDirection )
							particle.mVector = 
								frameMesh->GetMeshBuffer(j)->mMesh->Normal(k) / mNormalDirectionModifier;
						else
							particle.mVector = mDirection;

						particle.mStartTime = now;

						if( mMaxAngleDegrees )
						{
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
				}
			}
			else
			{
				int randomMB = 0;
				if( mMBNumber < 0 )
					randomMB = Randomizer::Rand() % mMBCount;
				else
					randomMB = mMBNumber;

				const MeshDescription& md = frameMesh->GetMeshBuffer(randomMB)->mMesh->GetDescription();
				unsigned int vertexNumber = md.mNumVertices;
				if (!vertexNumber)
					continue;
				vertexNumber = Randomizer::Rand() % vertexNumber;

				particle.mPos = frameMesh->GetMeshBuffer(randomMB)->mMesh->Position(vertexNumber);
				if( mUseNormalDirection )
					particle.mVector = frameMesh->GetMeshBuffer(randomMB)->mMesh->Normal(vertexNumber) / mNormalDirectionModifier;
				else
					particle.mVector = mDirection;

				particle.mStartTime = now;

				if( mMaxAngleDegrees )
				{
					Quaternion<float> tgt = Rotation<3, float>(
						AxisAngle<3, float>(mDirection, Randomizer::FRand() * mMaxAngleDegrees));
					mDirection = HProject(Rotate(tgt, Vector4<float> { 0.0f, 0.0f, 1.0f, 0.0f }));
					tgt = Rotation<3, float>(
						AxisAngle<3, float>(mDirection, Randomizer::FRand() * mMaxAngleDegrees));
					mDirection = HProject(Rotate(tgt, Vector4<float> { 1.0f, 0.0f, 0.0f, 0.0f }));
					tgt = Rotation<3, float>(
						AxisAngle<3, float>(mDirection, Randomizer::FRand() * mMaxAngleDegrees));
					mDirection = HProject(Rotate(tgt, Vector4<float> { 0.0f, 1.0f, 0.0f, 0.0f }));
					particle.mVector = mDirection;
				}

				particle.mEndTime = now + mMinLifeTime;
				if (mMaxLifeTime != mMinLifeTime)
					particle.mEndTime += Randomizer::Rand() % (mMaxLifeTime - mMinLifeTime);

				if (mMinStartColor==mMaxStartColor)
					particle.mColor = mMinStartColor;
				else
					particle.mColor = Function<float>::Lerp(mMinStartColor, mMaxStartColor, Randomizer::FRand());

				particle.mStartColor = particle.mColor;
				particle.mStartVector = particle.mVector;

				if (mMinStartSize == mMaxStartSize)
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
void ParticleAnimatedMeshNodeEmitter::SetAnimatedMeshNode( const eastl::shared_ptr<AnimatedMeshNode>& node )
{
	mAnimatedNode = node;
	mAnimatedMesh = 0;
	mBaseMesh = 0;
	mTotalVertices = 0;
	mVertexPerMeshBufferList.clear();
	if ( !mAnimatedNode ) return;

	mAnimatedMesh = mAnimatedNode->GetMesh();
	mBaseMesh = mAnimatedMesh->GetMesh(0);

	mMBCount = mBaseMesh->GetMeshBufferCount();
	for( unsigned int i = 0; i < mMBCount; ++i )
	{
		const MeshDescription& md = mBaseMesh->GetMeshBuffer(i)->mMesh->GetDescription();
		mVertexPerMeshBufferList.push_back(md.mNumVertices);
		mTotalVertices += md.mNumVertices;
	}
}