// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLEAFFECTOR_H
#define PARTICLEAFFECTOR_H

#include "Graphic/Effect/Particle.h"

//! Types of built in particle affectors
enum GRAPHIC_ITEM ParticleAffectorType
{
	PAT_NONE = 0,
	PAT_ATTRACT,
	PAT_FADE_OUT,
	PAT_GRAVITY,
	PAT_ROTATE,
	PAT_SCALE,
	PET_NONE,
	PAT_COUNT
};


//! A particle affector modifies particles.
class BaseParticleAffector
{
public:

	//! constructor
	BaseParticleAffector() : mEnabled(true) {}

	//! Affects an array of particles.
	/** \param now Current time. (Same as ITimer::getTime() would return)
	\param particlearray Array of particles.
	\param count Amount of particles in array. */
	virtual void Affect(unsigned int now, Particle* particlearray, unsigned int count) = 0;

	//! Sets whether or not the affector is currently enabled.
	virtual void SetEnabled(bool enabled) { mEnabled = enabled; }

	//! Gets whether or not the affector is currently enabled.
	virtual bool GetEnabled() const { return mEnabled; }

	//! Get emitter type
	virtual ParticleAffectorType GetType() const = 0;

protected:
	bool mEnabled;
};


#endif

