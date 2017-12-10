// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLEMESHEMITTER_H
#define PARTICLEMESHEMITTER_H

#include "ParticleEmitter.h"


//! A particle emitter which emits from vertices of a mesh
class ParticleMeshEmitter : public BaseParticleEmitter
{
public:

	//! constructor
	ParticleMeshEmitter(
		const eastl::shared_ptr<Mesh>& mesh, bool useNormalDirection = true,
		const Vector3<float>& direction = Vector3<float>{ 0.f, 0.f, 0.f },
		float normalDirectionModifier = 100.0f,  int mbNumber = -1, bool everyMeshVertex = false,
		unsigned int minParticlesPerSecond = 20, unsigned int maxParticlesPerSecond = 40,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{255.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		unsigned int lifeTimeMin = 2000, unsigned int lifeTimeMax = 4000, int maxAngleDegrees = 0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f }
	);

	//! Prepares an array with new particles to emitt into the system
	//! and returns how much new particles there are.
	virtual int Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray);

	//! Set Mesh to emit particles from
	virtual void SetMesh( const eastl::shared_ptr<Mesh>& mesh );

	//! Set whether to use vertex normal for direction, or direction specified
	virtual void SetUseNormalDirection( bool useNormalDirection ) { mUseNormalDirection = useNormalDirection; }

	//! Set direction the emitter emits particles
	virtual void SetDirection( const Vector3<float>& newDirection ) { mDirection = newDirection; }

	//! Set the amount that the normal is divided by for getting a particles direction
	virtual void SetNormalDirectionModifier( float normalDirectionModifier ) { mNormalDirectionModifier = normalDirectionModifier; }

	//! Sets whether to emit min<->max particles for every vertex per second, or to pick
	//! min<->max vertices every second
	virtual void SetEveryMeshVertex( bool everyMeshVertex ) { mEveryMeshVertex = everyMeshVertex; }

	//! Set minimum number of particles the emitter emits per second
	virtual void SetMinParticlesPerSecond( unsigned int minPPS ) { mMinParticlesPerSecond = minPPS; }

	//! Set maximum number of particles the emitter emits per second
	virtual void SetMaxParticlesPerSecond( unsigned int maxPPS ) { mMaxParticlesPerSecond = maxPPS; }

	//! Set minimum starting color for particles
	virtual void SetMinStartColor( const eastl::array<float, 4>& color ) { mMinStartColor = color; }

	//! Set maximum starting color for particles
	virtual void SetMaxStartColor( const eastl::array<float, 4>& color ) { mMaxStartColor = color; }

	//! Set the maximum starting size for particles
	virtual void SetMaxStartSize( const Vector2<float>& size ) { mMaxStartSize = size; }

	//! Set the minimum starting size for particles
	virtual void SetMinStartSize( const Vector2<float>& size ) { mMinStartSize = size; }

	//! Set the minimum particle life-time in milliseconds
	virtual void SetMinLifeTime( unsigned int lifeTimeMin ) { mMinLifeTime = lifeTimeMin; }

	//! Set the maximum particle life-time in milliseconds
	virtual void SetMaxLifeTime( unsigned int lifeTimeMax ) { mMaxLifeTime = lifeTimeMax; }

	//!	Set maximal random derivation from the direction
	virtual void SetMaxAngleDegrees( int maxAngleDegrees ) { mMaxAngleDegrees = maxAngleDegrees; }

	//! Get Mesh we're emitting particles from
	virtual const eastl::shared_ptr<Mesh>& GetMesh() const { return mParticleMesh; }

	//! Get whether to use vertex normal for direciton, or direction specified
	virtual bool IsUsingNormalDirection() const { return mUseNormalDirection; }

	//! Get direction the emitter emits particles
	virtual const Vector3<float>& GetDirection() const { return mDirection; }

	//! Get the amount that the normal is divided by for getting a particles direction
	virtual float GetNormalDirectionModifier() const { return mNormalDirectionModifier; }

	//! Gets whether to emit min<->max particles for every vertex per second, or to pick
	//! min<->max vertices every second
	virtual bool GetEveryMeshVertex() const { return mEveryMeshVertex; }

	//! Get the minimum number of particles the emitter emits per second
	virtual unsigned int GetMinParticlesPerSecond() const { return mMinParticlesPerSecond; }

	//! Get the maximum number of particles the emitter emits per second
	virtual unsigned int GetMaxParticlesPerSecond() const { return mMaxParticlesPerSecond; }

	//! Get the minimum starting color for particles
	virtual const eastl::array<float, 4>& GetMinStartColor() const { return mMinStartColor; }

	//! Get the maximum starting color for particles
	virtual const eastl::array<float, 4>& GetMaxStartColor() const { return mMaxStartColor; }

	//! Gets the maximum starting size for particles
	virtual const Vector2<float>& GetMaxStartSize() const { return mMaxStartSize; }

	//! Gets the minimum starting size for particles
	virtual const Vector2<float>& GetMinStartSize() const { return mMinStartSize; }

	//! Get the minimum particle life-time in milliseconds
	virtual unsigned int GetMinLifeTime() const { return mMinLifeTime; }

	//! Get the maximum particle life-time in milliseconds
	virtual unsigned int GetMaxLifeTime() const { return mMaxLifeTime; }

	//!	Get maximal random derivation from the direction
	virtual int GetMaxAngleDegrees() const { return mMaxAngleDegrees; }

private:

	eastl::shared_ptr<Mesh> mParticleMesh;
	eastl::vector<int> mVertexPerMeshBufferList;
	int mTotalVertices;
	unsigned int mMBCount;
	int mMBNumber;

	float mNormalDirectionModifier;
	eastl::vector<Particle> mParticles;
	Vector3<float> mDirection;
	Vector2<float> mMaxStartSize, mMinStartSize;
	unsigned int mMinParticlesPerSecond, mMaxParticlesPerSecond;
	eastl::array<float, 4> mMinStartColor, mMaxStartColor;
	unsigned int mMinLifeTime, mMaxLifeTime;

	unsigned int mTime;
	unsigned int mEmitted;
	int mMaxAngleDegrees;

	bool mEveryMeshVertex;
	bool mUseNormalDirection;
};


#endif // _PARTICLE_MESH_EMITTER_H_INCLUDED_

