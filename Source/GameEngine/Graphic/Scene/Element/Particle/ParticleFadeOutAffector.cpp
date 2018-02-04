// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleFadeOutAffector.h"

//! constructor
ParticleFadeOutAffector::ParticleFadeOutAffector(
	const eastl::array<float, 4>& targetColor, unsigned int fadeOutTime)
: BaseParticleAffector(), mTargetColor(targetColor)
{

	#ifdef _DEBUG
	//setDebugName("CParticleFadeOutAffector");
	#endif

	mFadeOutTime = fadeOutTime ? static_cast<float>(fadeOutTime) : 1.0f;
}


//! Affects an array of particles.
void ParticleFadeOutAffector::Affect(unsigned int now, Particle* particlearray, unsigned int count)
{
	if (!mEnabled)
		return;

	float d;

	for (unsigned int i=0; i<count; ++i)
	{
		if (particlearray[i].mEndTime - now < mFadeOutTime)
		{
			d = (particlearray[i].mEndTime - now) / mFadeOutTime;	// FadeOutTime probably float to save casts here (just guessing)
			particlearray[i].mColor = particlearray[i].mStartColor.GetInterpolated(mTargetColor, d);
		}
	}
}