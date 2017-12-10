// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef _PARTICLE_H_INCLUDED_
#define _PARTICLE_H_INCLUDED_

#include "Utilities/Vector3.h"
#include "Utilities/dimension2.h"
#include "Utilities/Color.h"

//! Struct for holding particle data
struct Particle
{
	//! Position of the particle
	Vector3<float> pos;

	//! Direction and speed of the particle
	Vector3<float> vector;

	//! Start life time of the particle
	unsigned int startTime;

	//! End life time of the particle
	unsigned int endTime;

	//! Current color of the particle
	Color color;

	//! Original color of the particle.
	/** That's the color of the particle it had when it was emitted. */
	Color startColor;

	//! Original direction and speed of the particle.
	/** The direction and speed the particle had when it was emitted. */
	Vector3<float> startVector;

	//! Scale of the particle.
	/** The current scale of the particle. */
	Vector2<float> size;

	//! Original scale of the particle.
	/** The scale of the particle when it was emitted. */
	Vector2<float> startSize;
};

#endif

