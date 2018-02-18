// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef PARTICLECYLINDEREMITTER_H
#define PARTICLECYLINDEREMITTER_H

#include "ParticleEmitter.h"

#include "Mathematic/Algebra/Vector2.h"

//! A default box emitter
class ParticleCylinderEmitter : public BaseParticleEmitter
{
public:

	//! constructor
	ParticleCylinderEmitter(
		const Vector3<float>& center, float radius, 
		const Vector3<float>& normal, float length, bool outlineOnly = false, 
		const Vector3<float>& direction = Vector3<float>{ 0.f, 0.03f, 0.f },
		unsigned int minParticlesPerSecond = 20, unsigned int maxParticlesPerSecond = 40,
		const eastl::array<float, 4>& minStartColor = eastl::array<float, 4>{255.f, 0.f, 0.f, 0.f},
		const eastl::array<float, 4>& maxStartColor = eastl::array<float, 4>{255.f, 255.f, 255.f, 255.f},
		unsigned int lifeTimeMin=2000, unsigned int lifeTimeMax=4000, int maxAngleDegrees=0,
		const Vector2<float>& minStartSize = Vector2<float>{ 5.f,5.f },
		const Vector2<float>& maxStartSize = Vector2<float>{ 5.f,5.f });

	//! Prepares an array with new particles to emitt into the system
	//! and returns how much new particles there are.
	virtual int Emitt(unsigned int now, unsigned int timeSinceLastCall, Particle*& outArray);

	//! Set the center of the radius for the cylinder, at one end of the cylinder
	virtual void SetCenter( const Vector3<float>& center ) { mCenter = center; }

	//! Set the normal of the cylinder
	virtual void SetNormal( const Vector3<float>& normal ) { mNormal = normal; }

	//! Set the radius of the cylinder
	virtual void SetRadius( float radius ) { mRadius = radius; }

	//! Set the length of the cylinder
	virtual void SetLength( float length ) { mLength = length; }

	//! Set whether or not to draw points inside the cylinder
	virtual void SetOutlineOnly( bool outlineOnly ) { mOutlineOnly = outlineOnly; }

	//! Set direction the emitter emits particles
	virtual void SetDirection( const Vector3<float>& newDirection ) { mDirection = newDirection; }

	//! Set direction the emitter emits particles
	virtual void SetMinParticlesPerSecond( unsigned int minPPS ) { mMinParticlesPerSecond = minPPS; }

	//! Set direction the emitter emits particles
	virtual void SetMaxParticlesPerSecond( unsigned int maxPPS ) { mMaxParticlesPerSecond = maxPPS; }

	//! Set direction the emitter emits particles
	virtual void SetMinStartColor( const eastl::array<float, 4>& color ) { mMinStartColor = color; }

	//! Set direction the emitter emits particles
	virtual void SetMaxStartColor( const eastl::array<float, 4>& color ) { mMaxStartColor = color; }

	//! Set the maximum starting size for particles
	virtual void SetMaxStartSize( const Vector2<float>& size ) { mMaxStartSize = size; }

	//! Set the minimum starting size for particles
	virtual void SetMinStartSize( const Vector2<float>& size ) { mMinStartSize = size; }

	//! Set the minimum particle life-time in milliseconds
	virtual void SetMinLifeTime( unsigned int lifeTimeMin ) { mMinLifeTime = lifeTimeMin; }

	//! Set the maximum particle life-time in milliseconds
	virtual void SetMaxLifeTime( unsigned int lifeTimeMax ) { mMaxLifeTime = lifeTimeMax; }

	//!	Maximal random derivation from the direction
	virtual void SetMaxAngleDegrees( int maxAngleDegrees ) { mMaxAngleDegrees = maxAngleDegrees; }

	//! Get the center of the cylinder
	virtual const Vector3<float>& GetCenter() const { return mCenter; }

	//! Get the normal of the cylinder
	virtual const Vector3<float>& GetNormal() const { return mNormal; }

	//! Get the radius of the cylinder
	virtual float GetRadius() const { return mRadius; }

	//! Get the center of the cylinder
	virtual float GetLength() const { return mLength; }

	//! Get whether or not to draw points inside the cylinder
	virtual bool GetOutlineOnly() const { return mOutlineOnly; }

	//! Gets direction the emitter emits particles
	virtual const Vector3<float>& GetDirection() const { return mDirection; }

	//! Gets direction the emitter emits particles
	virtual unsigned int GetMinParticlesPerSecond() const { return mMinParticlesPerSecond; }

	//! Gets direction the emitter emits particles
	virtual unsigned int GetMaxParticlesPerSecond() const { return mMaxParticlesPerSecond; }

	//! Gets direction the emitter emits particles
	virtual const eastl::array<float, 4>& GetMinStartColor() const { return mMinStartColor; }

	//! Gets direction the emitter emits particles
	virtual const eastl::array<float, 4>& GetMaxStartColor() const { return mMaxStartColor; }

	//! Gets the maximum starting size for particles
	virtual const Vector2<float>& GetMaxStartSize() const { return mMaxStartSize; }

	//! Gets the minimum starting size for particles
	virtual const Vector2<float>& GetMinStartSize() const { return mMinStartSize; }

	//! Get the minimum particle life-time in milliseconds
	virtual unsigned int GetMinLifeTime() const { return mMinLifeTime; }

	//! Get the maximum particle life-time in milliseconds
	virtual unsigned int GetMaxLifeTime() const { return mMaxLifeTime; }

	//!	Maximal random derivation from the direction
	virtual int GetMaxAngleDegrees() const { return mMaxAngleDegrees; }


private:

	eastl::vector<Particle> mParticles;

	Vector3<float>	mCenter;
	Vector3<float>	mNormal;
	Vector3<float>	mDirection;
	Vector2<float> mMaxStartSize, mMinStartSize;
	unsigned int mMinParticlesPerSecond, mMaxParticlesPerSecond;
	eastl::array<float, 4> mMinStartColor, mMaxStartColor;
	unsigned int mMinLifeTime, mMaxLifeTime;

	float mRadius;
	float mLength;

	unsigned int mTime;
	unsigned int mEmitted;
	int mMaxAngleDegrees;

	bool mOutlineOnly;
};

#endif

