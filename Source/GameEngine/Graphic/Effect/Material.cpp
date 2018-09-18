// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "Material.h"

Material::Material()
    : mType(MT_SOLID),
    mEmissive({ 0.0f, 0.0f, 0.0f, 1.0f }), mAmbient({ 1.0f, 1.0f, 1.0f, 1.0f }),
    mDiffuse({ 1.0f, 1.0f, 1.0f, 1.0f }), mSpecular({ 1.0f, 1.0f, 1.0f, 1.0f }), 
	mShininess(0.0f), mThickness(1.0f), mLighting(true), mShadingModel(SM_GOURAUD)
{
	mBlendState = eastl::make_shared<BlendState>();
	mRasterizerState = eastl::make_shared<RasterizerState>();
	mDepthStencilState = eastl::make_shared<DepthStencilState>();

	mRasterizerState->mEnableAntialiasedLine = true;
	mRasterizerState->mFillMode = RasterizerState::FILL_SOLID;
	mRasterizerState->mCullMode = RasterizerState::CULL_BACK;
	mRasterizerState->mEnableDepthClip = true;
	mDepthStencilState->mDepthEnable = true;
}

bool Material::IsTransparent() const
{
	return mType == MT_TRANSPARENT_ADD_COLOR ||
		mType == MT_TRANSPARENT_ALPHA_CHANNEL ||
		mType == MT_TRANSPARENT_VERTEX_ALPHA ||
		mType == MT_TRANSPARENT_REFLECTION_2_LAYER;
}

//! Inequality operator
/** \param b Material to compare to.
\return True if the materials differ, else false. */
bool Material::operator!=(const Material& other) const
{
	bool different =
		mType != other.mType ||
		mAmbient != other.mAmbient ||
		mDiffuse != other.mDiffuse ||
		mEmissive != other.mEmissive ||
		mSpecular != other.mSpecular ||
		mShininess != other.mShininess ||
		mThickness != other.mThickness ||
		mShadingModel != other.mShadingModel ||
		mLighting != other.mLighting ||
		mBlendState != other.mBlendState ||
		mRasterizerState != other.mRasterizerState ||
		mDepthStencilState != other.mDepthStencilState;
	for (unsigned int i = 0; (i<MATERIAL_MAX_TEXTURES) && !different; ++i)
	{
		different |= (mTextureLayer[i] != other.mTextureLayer[i]);
	}
	return different;
}

//! Equality operator
/** \param b Material to compare to.
\return True if the materials are equal, else false. */
bool Material::operator==(const Material& other) const
{
	return !(other != *this);
}

//! Gets the i-th texture
/** \param i The desired level.
\return Texture for texture level i, if defined, else 0. */
Texture2* Material::GetTexture(unsigned int i) const
{
	return i < MATERIAL_MAX_TEXTURES ? mTextureLayer[i].mTexture : 0;
}

//! Sets the i-th texture
/** If i>=MATERIAL_MAX_TEXTURES this setting will be ignored.
\param i The desired level.
\param tex Texture for texture level i. */
void Material::SetTexture(unsigned int i, Texture2* tex)
{
	if (i >= MATERIAL_MAX_TEXTURES)
		return;
	mTextureLayer[i].mTexture = tex;
}
