// Copyright (C) 2010-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLESCALEAFFECTOR_H
#define PARTICLESCALEAFFECTOR_H

#include "ParticleAffector.h"

#include "Mathematic/Algebra/Vector2.h"

class ParticleScaleAffector : public BaseParticleAffector
{

public:

	ParticleScaleAffector(const Vector2<float>& scaleTo = Vector2<float>{ 1.f, 1.f });

	virtual void Affect(unsigned int now, Particle *particlearray, unsigned int count);

	//! Get affector type
	virtual ParticleAffectorType GetType() const { return PAT_SCALE; }

protected:
	Vector2<float> mScaleTo;
};


#endif

