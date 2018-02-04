// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#ifndef LIGHTING_H
#define LIGHTING_H

#include "Graphic/GraphicStd.h"

#include "Mathematic/Algebra/Matrix4x4.h"

//! Enumeration for different types of lights
enum LightType
{
	//! point light, it has a position in space and radiates light in all directions
	LT_POINT,
	//! spot light, it has a position in space, a direction, and a limited cone of influence
	LT_SPOT,
	//! directional light, coming from a direction from an infinite distance
	LT_DIRECTIONAL,

	//! Only used for counting the elements of this enum
	LT_COUNT
};

struct GRAPHIC_ITEM Lighting
{
    // Construction.  The defaults are listed for each member.  The '*'
    // channels are not used but are included to match shader-constant
    // packing rules.
    Lighting();

    // (r,g,b,*): default (1,1,1,1)
    Vector4<float> mAmbient;

    // (r,g,b,*): default (1,1,1,1)
    Vector4<float> mDiffuse;

    // (r,g,b,*): default (1,1,1,1)
    Vector4<float> mSpecular;

    // (angle,cosAngle,sinAngle,exponent): default (pi/2,0,1,1)
    Vector4<float> mSpotCutoff;

    // Attenuation is: intensity/(constant + linear * (d + quadratic * d)
    // where d is the distance from the light position to the vertex position.
    // The distance is in model space.  If the transformation from model space
    // to world space involves uniform scaling, you can include the scaling
    // factor in the 'intensity' component (by multiplication).
    //
    // (constant,linear,quadratic,intensity): default (1,0,0,1)
    Vector4<float> mAttenuation;

	//! The angle of the spot's outer cone. Ignored for other lights.
	float mOuterCone;

	//! The angle of the spot's inner cone. Ignored for other lights.
	float mInnerCone;

	//! The light strength's decrease between Outer and Inner cone.
	float mFalloff;

	//! Read-ONLY! Position of the light.
	/** If Type is ELT_DIRECTIONAL, it is ignored. Changed via light scene node's position. */
	Vector3<float> mPosition;

	//! Read-ONLY! Direction of the light.
	/** If Type is ELT_POINT, it is ignored. Changed via light scene node's rotation. */
	Vector3<float> mDirection;

	//! Read-ONLY! Radius of light. Everything within this radius will be lighted.
	float mRadius;

	//! Read-ONLY! Type of the light. Default: ELT_POINT
	LightType mType;

	//! Read-ONLY! Does the light cast shadows?
	bool mCastShadows : 1;
};

#endif