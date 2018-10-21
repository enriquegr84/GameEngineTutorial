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
	mShininess(0.0f), mThickness(1.0f), mLighting(true), mAntiAliasing(true), 
	mDepthBuffer(true), mDepthMask(DepthStencilState::MASK_ALL),
	mFillMode(RasterizerState::FILL_SOLID),
	mCullMode(RasterizerState::CULL_NONE),
	mShadingModel(SM_GOURAUD)
{

}

bool Material::IsTransparent() const
{
	return mType == MT_TRANSPARENT;
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
		mAntiAliasing != other.mAntiAliasing ||
		mMultisampling != other.mMultisampling ||
		mBlendTarget != other.mBlendTarget ||
		mDepthBuffer != other.mDepthBuffer ||
		mFillMode != other.mFillMode ||
		mCullMode != other.mCullMode;
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
eastl::shared_ptr<Texture2> Material::GetTexture(unsigned int i) const
{
	return i < MATERIAL_MAX_TEXTURES ? mTextureLayer[i].mTexture : 0;
}

//! Sets the i-th texture
/** If i>=MATERIAL_MAX_TEXTURES this setting will be ignored.
\param i The desired level.
\param tex Texture for texture level i. */
void Material::SetTexture(unsigned int i, eastl::shared_ptr<Texture2> tex)
{
	if (i >= MATERIAL_MAX_TEXTURES)
		return;
	mTextureLayer[i].mTexture = tex;
}

void Material::Update(eastl::shared_ptr<BlendState>& blendState) const
{
	blendState->mTarget[0] = mBlendTarget;
}

void Material::Update(eastl::shared_ptr<RasterizerState>& rasterizerState) const
{
	rasterizerState->mCullMode = mCullMode;
	rasterizerState->mFillMode = mFillMode;
	rasterizerState->mAntiAliasing = mAntiAliasing;
	rasterizerState->mEnableMultisample = mMultisampling;
}

void Material::Update(eastl::shared_ptr<DepthStencilState>& depthStencilState) const
{
	depthStencilState->mDepthEnable = mDepthBuffer;
	depthStencilState->mWriteMask = mDepthMask;
}
