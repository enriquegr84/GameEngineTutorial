// Copyright (C) 2010-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleScaleAffector.h"

ParticleScaleAffector::ParticleScaleAffector(const Vector2<float>& scaleTo)
: mScaleTo(scaleTo)
{
	#ifdef _DEBUG
	//setDebugName("ParticleScaleAffector");
	#endif
}

void ParticleScaleAffector::Affect (unsigned int now, Particle *particlearray, unsigned int count)
{
	for(unsigned int i=0;i<count;i++)
	{
		const unsigned int maxdiff = particlearray[i].endTime - particlearray[i].startTime;
		const unsigned int curdiff = now - particlearray[i].startTime;
		const float newscale = (float)curdiff / maxdiff;
		particlearray[i].size = particlearray[i].startSize + mScaleTo*newscale;
	}
}

