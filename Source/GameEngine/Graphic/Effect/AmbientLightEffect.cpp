// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "AmbientLightEffect.h"

AmbientLightEffect::AmbientLightEffect(eastl::shared_ptr<ProgramFactory> const& factory,
    BufferUpdater const& updater, eastl::string path, eastl::shared_ptr<Material> const& material,
	eastl::shared_ptr<Lighting> const& lighting)
    :
    LightingEffect(factory, updater, path, material, lighting, nullptr)
{
    mMaterialConstant = eastl::make_shared<ConstantBuffer>(sizeof(InternalMaterial), true);
    UpdateMaterialConstant();
    mProgram->GetVShader()->Set("Material", mMaterialConstant);

    mLightingConstant = eastl::make_shared<ConstantBuffer>(sizeof(InternalLighting), true);
    UpdateLightingConstant();
    mProgram->GetVShader()->Set("Lighting", mLightingConstant);
}

void AmbientLightEffect::UpdateMaterialConstant()
{
    InternalMaterial* internalMaterial = mMaterialConstant->Get<InternalMaterial>();
    internalMaterial->emissive = mMaterial->emissive;
    internalMaterial->ambient = mMaterial->ambient;
    LightingEffect::UpdateMaterialConstant();
}

void AmbientLightEffect::UpdateLightingConstant()
{
    InternalLighting* internalLighting = mLightingConstant->Get<InternalLighting>();
    internalLighting->ambient = mLighting->ambient;
    internalLighting->attenuation = mLighting->attenuation;
    LightingEffect::UpdateLightingConstant();
}