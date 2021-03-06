// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "SpotLightTextureEffect.h"

SpotLightTextureEffect::SpotLightTextureEffect(eastl::shared_ptr<ProgramFactory> const& factory,
    BufferUpdater const& updater, eastl::vector<eastl::string> const& path, 
	eastl::shared_ptr<Material> const& material, eastl::shared_ptr<Lighting> const& lighting, 
	eastl::shared_ptr<LightCameraGeometry> const& geometry, eastl::shared_ptr<Texture2> const& texture, 
	SamplerState::Filter filter, SamplerState::Mode mode0, SamplerState::Mode mode1)
    :
    LightingEffect(factory, updater, path, material, lighting, geometry), mTexture(texture)
{
	mSampler = eastl::make_shared<SamplerState>();
	mSampler->mFilter = filter;
	mSampler->mMode[0] = mode0;
	mSampler->mMode[1] = mode1;

    mMaterialConstant = eastl::make_shared<ConstantBuffer>(sizeof(InternalMaterial), true);
    UpdateMaterialConstant();

    mLightingConstant = eastl::make_shared<ConstantBuffer>(sizeof(InternalLighting), true);
    UpdateLightingConstant();

    mGeometryConstant = eastl::make_shared<ConstantBuffer>(sizeof(InternalGeometry), true);
    UpdateGeometryConstant();

    mProgram->GetPShader()->Set("Material", mMaterialConstant);
    mProgram->GetPShader()->Set("Lighting", mLightingConstant);
    mProgram->GetPShader()->Set("LightCameraGeometry", mGeometryConstant);
#if defined(_OPENGL_)
	mProgram->GetPShader()->Set("baseSampler", mTexture);
#else
	mProgram->GetPShader()->Set("baseTexture", mTexture);
#endif
	mProgram->GetPShader()->Set("baseSampler", mSampler);
}

void SpotLightTextureEffect::SetTexture(eastl::shared_ptr<Texture2> const& texture)
{
	mTexture = texture;
#if defined(_OPENGL_)
	mProgram->GetPShader()->Set("baseSampler", mTexture);
#else
	mProgram->GetPShader()->Set("baseTexture", mTexture);
#endif
}

void SpotLightTextureEffect::UpdateMaterialConstant()
{
    InternalMaterial* internalMaterial = mMaterialConstant->Get<InternalMaterial>();
    internalMaterial->emissive = mMaterial->mEmissive;
    internalMaterial->ambient = mMaterial->mAmbient;
    internalMaterial->diffuse = mMaterial->mDiffuse;
    internalMaterial->specular = mMaterial->mSpecular;
    LightingEffect::UpdateMaterialConstant();
}

void SpotLightTextureEffect::UpdateLightingConstant()
{
    InternalLighting* internalLighting = mLightingConstant->Get<InternalLighting>();
    internalLighting->ambient = mLighting->mAmbient;
    internalLighting->diffuse = mLighting->mDiffuse;
    internalLighting->specular = mLighting->mSpecular;
    internalLighting->spotCutoff = mLighting->mSpotCutoff;
    internalLighting->attenuation = mLighting->mAttenuation;
    LightingEffect::UpdateLightingConstant();
}

void SpotLightTextureEffect::UpdateGeometryConstant()
{
    InternalGeometry* internalGeometry = mGeometryConstant->Get<InternalGeometry>();
    internalGeometry->lightModelPosition = mGeometry->lightModelPosition;
    internalGeometry->lightModelDirection = mGeometry->lightModelDirection;
    internalGeometry->cameraModelPosition = mGeometry->cameraModelPosition;
    LightingEffect::UpdateGeometryConstant();
}
