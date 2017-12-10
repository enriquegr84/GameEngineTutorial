// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleAnimatedMeshNodeEmitter.h"
#include "AnimatedMeshNode.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

//! constructor
ParticleAnimatedMeshNodeEmitter::ParticleAnimatedMeshNodeEmitter(
		const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, 
		const eastl::shared_ptr<AnimatedMeshNode>& node, bool useNormalDirection, 
		const Vector3<float>& direction, float normalDirectionModifier, int mbNumber, 
		bool everyMeshVertex, unsigned int minParticlesPerSecond, unsigned int maxParticlesPerSecond,
		const eastl::array<float, 4>& minStartColor, const eastl::array<float, 4>& maxStartColor, 
		unsigned int lifeTimeMin, unsigned int lifeTimeMax, int maxAngleDegrees, 
		const Vector2<float>& minStartSize, const Vector2<float>& maxStartSize)
:	Node(actorId, renderComponent, ERP_TRANSPARENT, ESNT_ANIMATED_MESH), mAnimatedNode(0), 
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
	SetAnimatedMeshNode(node);
}


//! Prepares an array with new particles to emitt into the system
//! and returns how much new particles there are.
int ParticleAnimatedMeshNodeEmitter::Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray)
{
	mTime += timeSinceLastCall;

	const unsigned int pps = (mMaxParticlesPerSecond - mMinParticlesPerSecond);
	const float perSecond = pps ? ((float)m_MinParticlesPerSecond + Randomizer::frand() * pps) : mMinParticlesPerSecond;
	const float everyWhatMillisecond = 1000.0f / perSecond;

	if(mTime > everyWhatMillisecond)
	{
		unsigned int amount = (unsigned int)((mTime / everyWhatMillisecond) + 0.5f);
		mTime = 0;
		Particle p;

		if(amount > mMaxParticlesPerSecond * 2)
			amount = mMaxParticlesPerSecond * 2;

		// Get Mesh for this frame
		const eastl::shared_ptr<Mesh>& frameMesh = 
			mAnimatedMesh->GetMesh(eastl::floor(mAnimatedNode->GetFrameNr()), 
			255, mAnimatedNode->GetStartFrame(), mAnimatedNode->GetEndFrame() );

		for(unsigned int i=0; i<amount; ++i)
		{
			if( mEveryMeshVertex )
			{
				for(unsigned int j=0; j<frameMesh->GetMeshBufferCount(); ++j )
				{
					for(unsigned int k=0; k<frameMesh->GetMeshBuffer(j)->GetVertexCount(); ++k )
					{
						p.pos = frameMesh->GetMeshBuffer(j)->GetPosition(k);
						if( mUseNormalDirection )
							p.vector = frameMesh->GetMeshBuffer(j)->GetNormal(k) /
								mNormalDirectionModifier;
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
				int randomMB = 0;
				if( mMBNumber < 0 )
					randomMB = Randomizer::rand() % mMBCount;
				else
					randomMB = mMBNumber;

				unsigned int vertexNumber = frameMesh->GetMeshBuffer(randomMB)->GetVertexCount();
				if (!vertexNumber)
					continue;
				vertexNumber = Randomizer::rand() % vertexNumber;

				p.pos = frameMesh->GetMeshBuffer(randomMB)->GetPosition(vertexNumber);
				if( mUseNormalDirection )
					p.vector = frameMesh->GetMeshBuffer(randomMB)->GetNormal(vertexNumber) / mNormalDirectionModifier;
				else
					p.vector = mDirection;

				p.startTime = now;

				if( mMaxAngleDegrees )
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

				if (mMinStartSize == mMaxStartSize)
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
void ParticleAnimatedMeshNodeEmitter::SetAnimatedMeshSceneNode( const eastl::shared_ptr<AnimatedMeshNode>& node )
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
		mVertexPerMeshBufferList.push_back( mBaseMesh->GetMeshBuffer(i)->GetVertexCount() );
		mTotalVertices += mBaseMesh->GetMeshBuffer(i)->GetVertexCount();
	}
}