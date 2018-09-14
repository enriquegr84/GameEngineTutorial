// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Material.h"

Material::Material()
    : mType(MT_SOLID), mFlag(MF_COLOR_MATERIAL),
    mEmissive({ 0.0f, 0.0f, 0.0f, 1.0f }),
    mAmbient({ 1.0f, 1.0f, 1.0f, 1.0f }),
    mDiffuse({ 1.0f, 1.0f, 1.0f, 1.0f }),
    mSpecular({ 1.0f, 1.0f, 1.0f, 1.0f }), 
	mThickness(1.0f), mShininess(0.0f)
{

}

//! Inequality operator
/** \param b Material to compare to.
\return True if the materials differ, else false. */
bool Material::operator!=(const Material& m) const
{
	bool different =
		mType != m.mType ||
		mFlag != m.mFlag;
	return different;
}

//! Equality operator
/** \param b Material to compare to.
\return True if the materials are equal, else false. */
bool Material::operator==(const Material& m) const
{
	return !(m != *this);
}

bool Material::IsTransparent() const
{
	return mType == MT_TRANSPARENT_ADD_COLOR ||
		mType == MT_TRANSPARENT_ALPHA_CHANNEL ||
		mType == MT_TRANSPARENT_VERTEX_ALPHA ||
		mType == MT_TRANSPARENT_REFLECTION_2_LAYER;
}
