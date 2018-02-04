// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLERINGEMITTER_H
#define PARTICLERINGEMITTER_H

#include "ParticleEmitter.h"

#include "Mathematic/Algebra/Vector2.h"

//! A ring emitter
class ParticleRingEmitter : public BaseParticleEmitter
{
public:

	//! constructor
	ParticleRingEmitter(
		const Vector3<float>& center, float radius, float ringThickness,
		const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		unsigned int minParticlesPerSecond = 20, unsigned int maxParticlesPerSecond = 40,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{255.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		unsigned int lifeTimeMin=2000, unsigned int lifeTimeMax=4000, int maxAngleDegrees=0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f }
		);

	//! Prepares an array with new particles to emitt into the system
	//! and returns how much new particles there are.
	virtual int Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray);

	//! Set direction the emitter emits particles
	virtual void SetDirection( const Vector3<float>& newDirection ) { mDirection = newDirection; }

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

	//! Set the center of the ring
	virtual void SetCenter( const Vector3<float>& center ) { mCenter = center; }

	//! Set the radius of the ring
	virtual void SetRadius( float radius ) { mRadius = radius; }

	//! Set the thickness of the ring
	virtual void SetRingThickness( float ringThickness ) { mRingThickness = ringThickness; }

	//! Gets direction the emitter emits particles
	virtual const Vector3<float>& GetDirection() const { return mDirection; }

	//! Gets the minimum number of particles the emitter emits per second
	virtual unsigned int GetMinParticlesPerSecond() const { return mMinParticlesPerSecond; }

	//! Gets the maximum number of particles the emitter emits per second
	virtual unsigned int GetMaxParticlesPerSecond() const { return mMaxParticlesPerSecond; }

	//! Gets the minimum starting color for particles
	virtual const eastl::array<float, 4>& GetMinStartColor() const { return mMinStartColor; }

	//! Gets the maximum starting color for particles
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

	//! Get the center of the ring
	virtual const Vector3<float>& GetCenter() const { return mCenter; }

	//! Get the radius of the ring
	virtual float GetRadius() const { return mRadius; }

	//! Get the thickness of the ring
	virtual float GetRingThickness() const { return mRingThickness; }

private:

	eastl::vector<Particle> mParticles;

	Vector3<float> mCenter;
	float mRadius;
	float mRingThickness;

	Vector3<float> mDirection;
	Vector2<float> mMaxStartSize, mMinStartSize;
	unsigned int mMinParticlesPerSecond, mMaxParticlesPerSecond;
	eastl::array<float, 4> mMinStartColor, mMaxStartColor;
	unsigned int mMinLifeTime, mMaxLifeTime;

	unsigned int mTime;
	unsigned int mEmitted;
	int mMaxAngleDegrees;
};

#endif

