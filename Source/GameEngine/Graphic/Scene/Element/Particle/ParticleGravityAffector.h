// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLEGRAVITYAFFECTOR_H
#define PARTICLEGRAVITYAFFECTOR_H

#include "ParticleAffector.h"


//! Particle Affector for affecting direction of particle
class ParticleGravityAffector : public BaseParticleAffector
{
public:

	ParticleGravityAffector( 
		const Vector3<float>& gravity = Vector3<float>{ 0.f, -0.03f, 0.f }, unsigned int timeForceLost = 1000);

	//! Affects a particle.
	virtual void Affect(unsigned int now, Particle* particlearray, unsigned int count);

	//! Set the time in milliseconds when the gravity force is totally
	//! lost and the particle does not move any more.
	virtual void SetTimeForceLost( float timeForceLost ) { mTimeForceLost = timeForceLost; }

	//! Set the direction and force of gravity.
	virtual void SetGravity( const Vector3<float>& gravity ) { mGravity = gravity; }

	//! Set the time in milliseconds when the gravity force is totally
	//! lost and the particle does not move any more.
	virtual float GetTimeForceLost() const { return mTimeForceLost; }

	//! Set the direction and force of gravity.
	virtual const Vector3<float>& GetGravity() const { return mGravity; }

	//! Get affector type
	virtual ParticleAffectorType GetType() const { return PAT_GRAVITY; }

private:
	float mTimeForceLost;
	Vector3<float> mGravity;
};


#endif

