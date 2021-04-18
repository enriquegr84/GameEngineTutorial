// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLEPOINTEMITTER_H
#define PARTICLEPOINTEMITTER_H

#include "ParticleEmitter.h"

//! A default point emitter
class ParticlePointEmitter : public BaseParticleEmitter
{
public:

	//! constructor
	ParticlePointEmitter(
		const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		unsigned int minParticlesPerSecond = 5, unsigned int maxParticlesPerSecond = 10,
		eastl::array<float, 4> minStartColor = eastl::array<float, 4>{255.f, 0.f, 0.f, 0.f},
		eastl::array<float, 4> maxStartColor = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		unsigned int lifeTimeMin=2000, unsigned int lifeTimeMax=4000, int maxAngleDegrees=0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f, 5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f, 5.f });

	//! Prepares an array with new particles to emitt into the system
	//! and returns how much new particles there are.
	virtual int Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray);

	//! Set direction the emitter emits particles.
	virtual void SetDirection( const Vector3<float>& newDirection ) { mDirection = newDirection; }

	//! Set minimum number of particles emitted per second.
	virtual void SetMinParticlesPerSecond( unsigned int minPPS ) { mMinParticlesPerSecond = minPPS; }

	//! Set maximum number of particles emitted per second.
	virtual void SetMaxParticlesPerSecond( unsigned int maxPPS ) { mMaxParticlesPerSecond = maxPPS; }

	//! Set minimum start color.
	virtual void SetMinStartColor( const eastl::array<float, 4>& color ) { mMinStartColor = color; }

	//! Set maximum start color.
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

	//! Gets direction the emitter emits particles.
	virtual const Vector3<float>& GetDirection() const { return mDirection; }

	//! Gets minimum number of particles emitted per second.
	virtual unsigned int GetMinParticlesPerSecond() const { return mMinParticlesPerSecond; }

	//! Gets maximum number of particles emitted per second.
	virtual unsigned int GetMaxParticlesPerSecond() const { return mMaxParticlesPerSecond; }

	//! Gets minimum start color.
	virtual const eastl::array<float, 4>& GetMinStartColor() const { return mMinStartColor; }

	//! Gets maximum start color.
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

	Particle mParticle;
	Vector3<float> mDirection;
	Vector2<float> mMinStartSize, mMaxStartSize;
	unsigned int mMinParticlesPerSecond, mMaxParticlesPerSecond;
	eastl::array<float, 4> mMinStartColor, mMaxStartColor;
	unsigned int mMinLifeTime, mMaxLifeTime;
	int mMaxAngleDegrees;

	unsigned int mTime;
	unsigned int mEmitted;
};

#endif

