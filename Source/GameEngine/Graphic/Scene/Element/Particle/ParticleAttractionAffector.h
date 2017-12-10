// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLEATTRACTIONAFFECTOR_H
#define PARTICLEATTRACTIONAFFECTOR_H

#include "ParticleAffector.h"

//! Particle Affector for attracting particles to a point
class ParticleAttractionAffector : public BaseParticleAffector
{
public:

	ParticleAttractionAffector(
		const Vector3<float>& point = Vector3<float>(), float speed = 1.0f,
		bool attract = true, bool affectX = true,
		bool affectY = true, bool affectZ = true );

	//! Affects a particle.
	virtual void Affect(unsigned int now, Particle* particlearray, unsigned int count);

	//! Set the point that particles will attract to
	virtual void SetPoint( const Vector3<float>& point ) { mPoint = point; }

	//! Set the speed, in game units per second that the particles will attract to the specified point
	virtual void SetSpeed( float speed ) { mSpeed = speed; }

	//! Set whether or not the particles are attracting or detracting
	virtual void SetAttract( bool attract ) { mAttract = attract; }

	//! Set whether or not this will affect particles in the X direction
	virtual void SetAffectX( bool affect ) { mAffectX = affect; }

	//! Set whether or not this will affect particles in the Y direction
	virtual void SetAffectY( bool affect ) { mAffectY = affect; }

	//! Set whether or not this will affect particles in the Z direction
	virtual void SetAffectZ( bool affect ) { mAffectZ = affect; }

	//! Get the point that particles are attracted to
	virtual const Vector3<float>& GetPoint() const { return mPoint; }

	//! Get the speed that points attract to the specified point
	virtual float GetSpeed() const { return mSpeed; }

	//! Get whether or not the particles are attracting or detracting
	virtual bool GetAttract() const { return mAttract; }

	//! Get whether or not the particles X position are affected
	virtual bool GetAffectX() const { return mAffectX; }

	//! Get whether or not the particles Y position are affected
	virtual bool GetAffectY() const { return mAffectY; }

	//! Get whether or not the particles Z position are affected
	virtual bool GetAffectZ() const { return mAffectZ; }

	//! Get affector type
	virtual ParticleAffectorType GetType() const { return PAT_ATTRACT; }

private:

	Vector3<float> mPoint;
	float mSpeed;
	bool mAffectX;
	bool mAffectY;
	bool mAffectZ;
	bool mAttract;
	unsigned int mLastTime;
};

#endif

