// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLEROTATIONAFFECTOR_H
#define PARTICLEROTATIONAFFECTOR_H

#include "ParticleAffector.h"

//! Particle Affector for rotating particles about a point
class ParticleRotationAffector : public BaseParticleAffector
{
public:

	ParticleRotationAffector( 
		const Vector3<float>& speed = Vector3<float>{ 5.f, 5.f, 5.f },
		const Vector3<float>& point = Vector3<float>() 
	);

	//! Affects a particle.
	virtual void Affect(unsigned int now, Particle* particlearray, unsigned int count);

	//! Set the point that particles will attract to
	virtual void SetPivotPoint( const Vector3<float>& point ) { m_PivotPoint = point; }

	//! Set the speed in degrees per second
	virtual void SetSpeed( const Vector3<float>& speed ) { m_Speed = speed; }

	//! Get the point that particles are attracted to
	virtual const Vector3<float>& GetPivotPoint() const { return m_PivotPoint; }

	//! Get the speed in degrees per second
	virtual const Vector3<float>& GetSpeed() const { return m_Speed; }

	//! Get affector type
	virtual ParticleAffectorType GetType() const { return PAT_ROTATE; }

private:

	Vector3<float> mPivotPoint;
	Vector3<float> mSpeed;
	unsigned int mLastTime;
};

#endif

