// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLEEMITTER_H
#define PARTICLEEMITTER_H

#include "Graphic/Effect/Particle.h"

//! Types of built in particle emitters
enum GRAPHIC_ITEM ParticleEmitterType
{
	PET_POINT = 0,
	PET_ANIMATED_MESH,
	PET_BOX,
	PET_CYLINDER,
	PET_MESH,
	PET_RING,
	PET_SPHERE,
	PET_COUNT
};

//! A particle emitter for using with particle systems.
/** A Particle emitter emitts new particles into a particle system.
*/
class BaseParticleEmitter
{
public:

	//! Prepares an array with new particles to emitt into the system
	/** \param now Current time.
	\param timeSinceLastCall Time elapsed since last call, in milliseconds.
	\param outArray Pointer which will point to the array with the new
	particles to add into the system.
	\return Amount of new particles in the array. Can be 0. */
	virtual int Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray) = 0;

	//! Set direction the emitter emits particles
	virtual void SetDirection( const Vector3<float>& newDirection ) = 0;

	//! Set minimum number of particles the emitter emits per second
	virtual void SetMinParticlesPerSecond( unsigned int minPPS ) = 0;

	//! Set maximum number of particles the emitter emits per second
	virtual void SetMaxParticlesPerSecond( unsigned int maxPPS ) = 0;

	//! Set minimum starting color for particles
	virtual void SetMinStartColor( const SColorF& color ) = 0;

	//! Set maximum starting color for particles
	virtual void SetMaxStartColor( const SColorF& color ) = 0;

	//! Set the maximum starting size for particles
	virtual void SetMaxStartSize( const Vector2<float>& size ) = 0;

	//! Set the minimum starting size for particles
	virtual void SetMinStartSize( const Vector2<float>& size ) = 0;

	//! Set the minimum particle life-time in milliseconds
	virtual void SetMinLifeTime( unsigned int lifeTimeMin ) = 0;

	//! Set the maximum particle life-time in milliseconds
	virtual void SetMaxLifeTime( unsigned int lifeTimeMax ) = 0;

	//! Set maximal random derivation from the direction
	virtual void SetMaxAngleDegrees( int maxAngleDegrees ) = 0;

	//! Get direction the emitter emits particles
	virtual const Vector3<float>& GetDirection() const = 0;

	//! Get the minimum number of particles the emitter emits per second
	virtual unsigned int GetMinParticlesPerSecond() const = 0;

	//! Get the maximum number of particles the emitter emits per second
	virtual unsigned int GetMaxParticlesPerSecond() const = 0;

	//! Get the minimum starting color for particles
	virtual const SColorF& GetMinStartColor() const = 0;

	//! Get the maximum starting color for particles
	virtual const SColorF& GetMaxStartColor() const = 0;

	//! Get the maximum starting size for particles
	virtual const Vector2<float>& GetMaxStartSize() const = 0;

	//! Get the minimum starting size for particles
	virtual const Vector2<float>& GetMinStartSize() const = 0;

	//! Get the minimum particle life-time in milliseconds
	virtual unsigned int GetMinLifeTime() const = 0;

	//! Get the maximum particle life-time in milliseconds
	virtual unsigned int GetMaxLifeTime() const = 0;

	//! Get maximal random derivation from the direction
	virtual int GetMaxAngleDegrees() const = 0;

	//! Get emitter type
	virtual ParticleEmitterType GetType() const { return PET_POINT; }
};

typedef BaseParticleEmitter BaseParticlePointEmitter;

#endif

