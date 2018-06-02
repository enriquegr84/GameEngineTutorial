// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleAttractionAffector.h"

//! constructor
ParticleAttractionAffector::ParticleAttractionAffector( const Vector3<float>& point, 
	float speed, bool attract, bool affectX, bool affectY, bool affectZ )
:	mPoint(point), mSpeed(speed), mAffectX(affectX), mAffectY(affectY),
	mAffectZ(affectZ), mAttract(attract), mLastTime(0)
{
	#ifdef _DEBUG
	//setDebugName("ParticleAttractionAffector");
	#endif
}


//! Affects an array of particles.
void ParticleAttractionAffector::Affect(unsigned int now, Particle* particlearray, unsigned int count)
{
	if( mLastTime == 0 )
	{
		mLastTime = now;
		return;
	}

	float timeDelta = ( now - mLastTime ) / 1000.0f;
	mLastTime = now;

	if( !mEnabled )
		return;

	for(unsigned int i=0; i<count; ++i)
	{
		Vector3<float> direction(mPoint - particlearray[i].mPos);
		Normalize(direction);
		direction *= mSpeed * timeDelta;

		if( !mAttract )
			direction *= -1.0f;

		if( mAffectX )
			particlearray[i].mPos[0] += direction[0];

		if( mAffectY )
			particlearray[i].mPos[1] += direction[1];

		if( mAffectZ )
			particlearray[i].mPos[2] += direction[2];
	}
}