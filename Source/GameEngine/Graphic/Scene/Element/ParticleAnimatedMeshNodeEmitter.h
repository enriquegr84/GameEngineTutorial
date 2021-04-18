// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLEANIMATEDMESHNODEEMITTER_H
#define PARTICLEANIMATEDMESHNODEEMITTER_H

#include "AnimatedMeshNode.h"

#include "Graphic/Scene/Hierarchy/Node.h"

#include "Graphic/Effect/Particle.h"

//! An animated mesh emitter
class ParticleAnimatedMeshNodeEmitter : public Node
{
public:

	//! constructor
	ParticleAnimatedMeshNodeEmitter(
		const ActorId actorId, PVWUpdater* updater,
		const eastl::shared_ptr<AnimatedMeshNode>& node,
		bool useNormalDirection = true,
		const Vector3<float>& direction = Vector3<float>{ 0.f,0.f,-1.f },
		float normalDirectionModifier = 100.0f,
		int mbNumber = -1,
		bool everyMeshVertex = false,
		unsigned int minParticlesPerSecond = 20,
		unsigned int maxParticlesPerSecond = 40,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{255.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		unsigned int lifeTimeMin = 2000,
		unsigned int lifeTimeMax = 4000,
		int maxAngleDegrees = 0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Prepares an array with new particles to emitt into the system
	//! and returns how much new particles there are.
	int Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray);

	//! Set Mesh to emit particles from
	void SetAnimatedMeshNode( const eastl::shared_ptr<AnimatedMeshNode>& node );

	//! Set whether to use vertex normal for direction, or direction specified
	void SetUseNormalDirection( bool useNormalDirection ) { mUseNormalDirection = useNormalDirection; }

	//! Set direction the emitter emits particles
	void SetDirection( const Vector3<float>& newDirection ) { mDirection = newDirection; }

	//! Set the amount that the normal is divided by for getting a particles direction
	void SetNormalDirectionModifier( float normalDirectionModifier ) { mNormalDirectionModifier = normalDirectionModifier; }

	//! Sets whether to emit min<->max particles for every vertex per second, or to pick
	//! min<->max vertices every second
	void SetEveryMeshVertex( bool everyMeshVertex ) { mEveryMeshVertex = everyMeshVertex; }

	//! Set minimum number of particles the emitter emits per second
	void SetMinParticlesPerSecond( unsigned int minPPS ) { mMinParticlesPerSecond = minPPS; }

	//! Set maximum number of particles the emitter emits per second
	void SetMaxParticlesPerSecond( unsigned int maxPPS ) { mMaxParticlesPerSecond = maxPPS; }

	//! Set minimum starting color for particles
	void SetMinStartColor( const eastl::array<float, 4>& color ) { mMinStartColor = color; }

	//! Set maximum starting color for particles
	void SetMaxStartColor( const eastl::array<float, 4>& color ) { mMaxStartColor = color; }

	//! Set the maximum starting size for particles
	void SetMaxStartSize( const Vector2<float>& size ) { mMaxStartSize = size; }

	//! Set the minimum starting size for particles
	void SetMinStartSize( const Vector2<float>& size ) { mMinStartSize = size; }

	//! Set the minimum particle life-time in milliseconds
	void SetMinLifeTime( unsigned int lifeTimeMin ) { mMinLifeTime = lifeTimeMin; }

	//! Set the maximum particle life-time in milliseconds
	void SetMaxLifeTime( unsigned int lifeTimeMax ) { mMaxLifeTime = lifeTimeMax; }

	//!	Maximal random derivation from the direction
	void SetMaxAngleDegrees( int maxAngleDegrees ) { mMaxAngleDegrees = maxAngleDegrees; }

	//! Get Mesh we're emitting particles from
	const eastl::shared_ptr<AnimatedMeshNode>& GetAnimatedMeshNode() const { return mAnimatedNode; }

	//! Get whether to use vertex normal for direciton, or direction specified
	bool IsUsingNormalDirection() const { return mUseNormalDirection; }

	//! Get direction the emitter emits particles
	const Vector3<float>& GetDirection() const { return mDirection; }

	//! Get the amount that the normal is divided by for getting a particles direction
	float GetNormalDirectionModifier() const { return mNormalDirectionModifier; }

	//! Gets whether to emit min<->max particles for every vertex per second, or to pick
	//! min<->max vertices every second
	bool GetEveryMeshVertex() const { return mEveryMeshVertex; }

	//! Get the minimum number of particles the emitter emits per second
	unsigned int GetMinParticlesPerSecond() const { return mMinParticlesPerSecond; }

	//! Get the maximum number of particles the emitter emits per second
	unsigned int GetMaxParticlesPerSecond() const { return mMaxParticlesPerSecond; }

	//! Get the minimum starting color for particles
	const eastl::array<float, 4>& GetMinStartColor() const { return mMinStartColor; }

	//! Get the maximum starting color for particles
	const eastl::array<float, 4>& GetMaxStartColor() const { return mMaxStartColor; }

	//! Get the maximum starting size for particles
	const Vector2<float>& GetMaxStartSize() const { return mMaxStartSize; }

	//! Get the minimum starting size for particles
	const Vector2<float>& GetMinStartSize() const { return mMinStartSize; }

	//! Get the minimum particle life-time in milliseconds
	unsigned int GetMinLifeTime() const { return mMinLifeTime; }

	//! Get the maximum particle life-time in milliseconds
	unsigned int GetMaxLifeTime() const { return mMaxLifeTime; }

	//!	Maximal random derivation from the direction
	int GetMaxAngleDegrees() const { return mMaxAngleDegrees; }

private:

	eastl::shared_ptr<AnimatedMeshNode>		mAnimatedNode;
	eastl::shared_ptr<BaseAnimatedMesh>		mAnimatedMesh;
	eastl::shared_ptr<BaseMesh>				mBaseMesh;
	int						mTotalVertices;
	unsigned int			mMBCount;
	int						mMBNumber;
	eastl::vector<int>		mVertexPerMeshBufferList;
	eastl::vector<Particle> mParticles;

	Vector3<float>			mDirection;
	float					mNormalDirectionModifier;
	unsigned int			mMinParticlesPerSecond, mMaxParticlesPerSecond;
	eastl::array<float, 4>	mMinStartColor, mMaxStartColor;
	unsigned int			mMinLifeTime, mMaxLifeTime;
	Vector2<float>			mMaxStartSize, mMinStartSize;

	unsigned int		mTime;
	unsigned int		mEmitted;
	int					mMaxAngleDegrees;

	bool	mEveryMeshVertex;
	bool	mUseNormalDirection;
};

#endif // _PARTICLEANIMATEDMESHSCENENODEEMITTER_H_INCLUDED_

