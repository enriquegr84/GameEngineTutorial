// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "SpotLightEffect.h"

SpotLightEffect::SpotLightEffect(eastl::shared_ptr<ProgramFactory> const& factory,
    BufferUpdater const& updater, eastl::string path, int select, eastl::shared_ptr<Material> const& material,
	eastl::shared_ptr<Lighting> const& lighting, eastl::shared_ptr<LightCameraGeometry> const& geometry)
    :
    LightingEffect(factory, updater, path, material, lighting, geometry)
{
    mMaterialConstant = eastl::make_shared<ConstantBuffer>(sizeof(InternalMaterial), true);
    UpdateMaterialConstant();

    mLightingConstant = eastl::make_shared<ConstantBuffer>(sizeof(InternalLighting), true);
    UpdateLightingConstant();

    mGeometryConstant = eastl::make_shared<ConstantBuffer>(sizeof(InternalGeometry), true);
    UpdateGeometryConstant();

    if ((select & 1) == 0)
    {
        mProgram->GetVShader()->Set("Material", mMaterialConstant);
        mProgram->GetVShader()->Set("Lighting", mLightingConstant);
        mProgram->GetVShader()->Set("LightCameraGeometry", mGeometryConstant);
    }
    else
    {
        mProgram->GetPShader()->Set("Material", mMaterialConstant);
        mProgram->GetPShader()->Set("Lighting", mLightingConstant);
        mProgram->GetPShader()->Set("LightCameraGeometry", mGeometryConstant);
    }
}

void SpotLightEffect::UpdateMaterialConstant()
{
    InternalMaterial* internalMaterial = mMaterialConstant->Get<InternalMaterial>();
    internalMaterial->emissive = mMaterial->mEmissive;
    internalMaterial->ambient = mMaterial->mAmbient;
    internalMaterial->diffuse = mMaterial->mDiffuse;
    internalMaterial->specular = mMaterial->mSpecular;
    LightingEffect::UpdateMaterialConstant();
}

void SpotLightEffect::UpdateLightingConstant()
{
    InternalLighting* internalLighting = mLightingConstant->Get<InternalLighting>();
    internalLighting->ambient = mLighting->mAmbient;
    internalLighting->diffuse = mLighting->mDiffuse;
    internalLighting->specular = mLighting->mSpecular;
    internalLighting->spotCutoff = mLighting->mSpotCutoff;
    internalLighting->attenuation = mLighting->mAttenuation;
    LightingEffect::UpdateLightingConstant();
}

void SpotLightEffect::UpdateGeometryConstant()
{
    InternalGeometry* internalGeometry = mGeometryConstant->Get<InternalGeometry>();
    internalGeometry->lightModelPosition = mGeometry->lightModelPosition;
    internalGeometry->lightModelDirection = mGeometry->lightModelDirection;
    internalGeometry->cameraModelPosition = mGeometry->cameraModelPosition;
    LightingEffect::UpdateGeometryConstant();
}
