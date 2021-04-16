// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLEBOXEMITTER_H
#define PARTICLEBOXEMITTER_H

#include "ParticleEmitter.h"

#include "Mathematic/Geometric/AlignedBox.h"

//! A default box emitter
class ParticleBoxEmitter : public BaseParticleEmitter
{
public:

	//! constructor
	ParticleBoxEmitter( const AlignedBox3<float>& box,
		const Vector3<float>& direction = Vector3<float>{ 0.0f, 0.03f, 0.f },
		unsigned int minParticlesPerSecond = 20, unsigned int maxParticlesPerSecond = 40,
        SColorF minStartColor = SColorF(0, 0, 0),
        SColorF maxStartColor = SColorF(1.f, 1.f, 1.f),
		unsigned int lifeTimeMin=2000, unsigned int lifeTimeMax=4000, int maxAngleDegrees=0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f,5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f,5.f });

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
	virtual void SetMinStartColor( const SColorF& color ) { mMinStartColor = color; }

	//! Set maximum start color.
	virtual void SetMaxStartColor( const SColorF& color ) { mMaxStartColor = color; }

	//! Set the maximum starting size for particles
	virtual void SetMaxStartSize( const Vector2<float>& size ) { mMaxStartSize = size; }

	//! Set the minimum starting size for particles
	virtual void SetMinStartSize( const Vector2<float>& size ) { mMinStartSize = size; }

	//! Set the minimum particle life-time in milliseconds
	virtual void SetMinLifeTime( unsigned int lifeTimeMin ) { mMinLifeTime = lifeTimeMin; }

	//! Set the maximum particle life-time in milliseconds
	virtual void SetMaxLifeTime( unsigned int lifeTimeMax ) { mMaxLifeTime = lifeTimeMax; }

	//!	Maximal random derivation from the direction
	virtual void SetMaxAngleDegrees( int maxAngleDegrees ) { mMaxAngleDegrees = maxAngleDegrees; }

	//! Set box from which the particles are emitted.
	virtual void SetBox( const AlignedBox3<float>& box ) { mBox = box; }

	//! Gets direction the emitter emits particles.
	virtual const Vector3<float>& GetDirection() const { return mDirection; }

	//! Gets minimum number of particles emitted per second.
	virtual unsigned int GetMinParticlesPerSecond() const { return mMinParticlesPerSecond; }

	//! Gets maximum number of particles emitted per second.
	virtual unsigned int GetMaxParticlesPerSecond() const { return mMaxParticlesPerSecond; }

	//! Gets minimum start color.
	virtual const SColorF& GetMinStartColor() const { return mMinStartColor; }

	//! Gets maximum start color.
	virtual const SColorF& GetMaxStartColor() const { return mMaxStartColor; }

	//! Gets the maximum starting size for particles
	virtual const Vector2<float>& GetMaxStartSize() const { return mMaxStartSize; }

	//! Gets the minimum starting size for particles
	virtual const Vector2<float>& GetMinStartSize() const { return mMinStartSize; }

	//! Get the minimum particle life-time in milliseconds
	virtual unsigned int GetMinLifeTime() const { return mMinLifeTime; }

	//! Get the maximum particle life-time in milliseconds
	virtual unsigned int GetMaxLifeTime() const { return mMaxLifeTime; }

	//!	Maximal random derivation from the direction
	virtual int GetMaxAngleDegrees() const { return mMaxAngleDegrees; }

	//! Get box from which the particles are emitted.
	virtual const AlignedBox3<float>& GetBox() const { return mBox; }

private:

	eastl::vector<Particle> mParticles;
	AlignedBox3<float> mBox;
	Vector3<float> mDirection;
	Vector2<float> mMaxStartSize, mMinStartSize;
	unsigned int mMinParticlesPerSecond, mMaxParticlesPerSecond;
    SColorF mMinStartColor, mMaxStartColor;
	unsigned int mMinLifeTime, mMaxLifeTime;

	unsigned int mTime;
	unsigned int mEmitted;
	int mMaxAngleDegrees;
};

#endif

