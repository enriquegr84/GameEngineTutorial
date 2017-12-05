// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/11/13)

#include "LightingEffect.h"

#include "Mathematic/Algebra/Matrix4x4.h"

LightingEffect::LightingEffect(eastl::shared_ptr<ProgramFactory> const& factory,
    BufferUpdater const& updater, eastl::string path, eastl::shared_ptr<Material> const& material, 
	eastl::shared_ptr<Lighting> const& lighting, eastl::shared_ptr<LightCameraGeometry> const& geometry)
    :
    mMaterial(material),
    mLighting(lighting),
    mGeometry(geometry)
{
	mProgram = factory->CreateFromFiles(path, path, "");
    if (mProgram)
    {
        mBufferUpdater = updater;
        mPVWMatrixConstant = eastl::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
        mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
    }
}

void LightingEffect::SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& pvwMatrix)
{
    mPVWMatrixConstant = pvwMatrix;
    mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}

void LightingEffect::UpdateMaterialConstant()
{
    if (mMaterialConstant)
    {
        mBufferUpdater(mMaterialConstant);
    }
}

void LightingEffect::UpdateLightingConstant()
{
    if (mLightingConstant)
    {
        mBufferUpdater(mLightingConstant);
    }
}

void LightingEffect::UpdateGeometryConstant()
{
    if (mGeometryConstant)
    {
        mBufferUpdater(mGeometryConstant);
    }
}