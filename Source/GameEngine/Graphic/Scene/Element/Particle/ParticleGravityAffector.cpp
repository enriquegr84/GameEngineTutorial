// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleGravityAffector.h"

#include "Graphic/Scene/Scene.h"

//! constructor
ParticleGravityAffector::ParticleGravityAffector(
	const Vector3<float>& gravity, unsigned int timeForceLost)
	: BaseParticleAffector(), mTimeForceLost(static_cast<float>(timeForceLost)), mGravity(gravity)
{

}


//! Affects an array of particles.
void ParticleGravityAffector::Affect(unsigned int now, Particle* particlearray, unsigned int count)
{
	if (!mEnabled)
		return;

	float d;

	for (unsigned int i=0; i<count; ++i)
	{
		d = (now - particlearray[i].mStartTime) / mTimeForceLost;
		if (d > 1.0f)
			d = 1.0f;
		if (d < 0.0f)
			d = 0.0f;
		d = 1.0f - d;

		particlearray[i].mVector = Function<float>::Lerp(particlearray[i].mStartVector, mGravity, d);
	}
}