// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLE_H
#define PARTICLE_H

#include "Core/CoreStd.h"
#include "Graphic/GraphicStd.h"

#include "Graphic/Resource/Color.h"

#include "Mathematic/Algebra/Vector3.h"
#include "Mathematic/Algebra/Vector2.h"

//! Struct for holding particle data
struct GRAPHIC_ITEM Particle
{
	//! Position of the particle
	Vector3<float> mPos;

	//! Direction and speed of the particle
	Vector3<float> mVector;

	//! Start life time of the particle
	unsigned int mStartTime;

	//! End life time of the particle
	unsigned int mEndTime;

	//! Current color of the particle
    SColorF mColor;

	//! Original color of the particle.
	/** That's the color of the particle it had when it was emitted. */
    SColorF mStartColor;

	//! Original direction and speed of the particle.
	/** The direction and speed the particle had when it was emitted. */
	Vector3<float> mStartVector;

	//! Scale of the particle.
	/** The current scale of the particle. */
	Vector2<float> mSize;

	//! Original scale of the particle.
	/** The scale of the particle when it was emitted. */
	Vector2<float> mStartSize;
};

#endif

