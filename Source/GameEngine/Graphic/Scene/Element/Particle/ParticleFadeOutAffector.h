// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLEFADEOUTAFFECTOR_H
#define PARTICLEFADEOUTAFFECTOR_H

#include "ParticleAffector.h"

//! Particle Affector for fading out a color
class ParticleFadeOutAffector : public BaseParticleAffector
{
public:

	ParticleFadeOutAffector(const SColorF& targetColor, unsigned int fadeOutTime);

	//! Affects a particle.
	virtual void Affect(unsigned int now, Particle* particlearray, unsigned int count);

	//! Sets the targetColor, i.e. the color the particles will interpolate
	//! to over time.
	virtual void SetTargetColor( const SColorF& targetColor ) { mTargetColor = targetColor; }

	//! Sets the amount of time it takes for each particle to fade out.
	virtual void SetFadeOutTime( unsigned int fadeOutTime ) { mFadeOutTime = fadeOutTime ? static_cast<float>(fadeOutTime) : 1.0f; }

	//! Sets the targetColor, i.e. the color the particles will interpolate
	//! to over time.
	virtual const SColorF& GetTargetColor() const { return mTargetColor; }

	//! Sets the amount of time it takes for each particle to fade out.
	virtual unsigned int GetFadeOutTime() const { return static_cast<unsigned int>(mFadeOutTime); }

	//! Get affector type
	virtual ParticleAffectorType GetType() const { return PAT_FADE_OUT; }

private:

    SColorF mTargetColor;
	float mFadeOutTime;
};

#endif

