// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "ParticleRotationAffector.h"

#include "Graphic/Renderer/Renderer.h"
#include "Graphic/Effect/Material.h"

#include "Core/OS/OS.h"

#include "Graphic/Scene/Scene.h"

//! constructor
ParticleRotationAffector::ParticleRotationAffector( const Vector3<float>& speed, const Vector3<float>& pivotPoint )
: mPivotPoint(pivotPoint), mSpeed(speed), mLastTime(0)
{
	#ifdef _DEBUG
	//setDebugName("CParticleRotationAffector");
	#endif
}


//! Affects an array of particles.
void ParticleRotationAffector::Affect(unsigned int now, Particle* particlearray, unsigned int count)
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
		if (mSpeed[0] != 0.0f)
		{
			Quaternion<float> tgt = Rotation<3, float>(
				AxisAngle<3, float>(particlearray[i].mPos, timeDelta * mSpeed[0]));
			particlearray[i].mPos = HProject(Rotate(tgt, Vector4<float> { 1.0f, 0.0f, 0.0f, 0.0f }));
			//particlearray[i].mPos.RotateYZBy(timeDelta * mSpeed.X, mPivotPoint);
		}

		if (mSpeed[1] != 0.0f)
		{
			Quaternion<float> tgt = Rotation<3, float>(
				AxisAngle<3, float>(particlearray[i].mPos, timeDelta * mSpeed[1]));
			particlearray[i].mPos = HProject(Rotate(tgt, Vector4<float> { 0.0f, 1.0f, 0.0f, 0.0f }));
			//particlearray[i].mPos.RotateXZBy(timeDelta * mSpeed.Y, mPivotPoint);
		}

		if (mSpeed[2] != 0.0f)
		{
			Quaternion<float> tgt = Rotation<3, float>(
				AxisAngle<3, float>(particlearray[i].mPos, timeDelta * mSpeed[2]));
			particlearray[i].mPos = HProject(Rotate(tgt, Vector4<float> { 0.0f, 0.0f, 1.0f, 0.0f }));
			//particlearray[i].mPos.RotateXYBy(timeDelta * mSpeed.Z, mPivotPoint);
		}
	}
}
