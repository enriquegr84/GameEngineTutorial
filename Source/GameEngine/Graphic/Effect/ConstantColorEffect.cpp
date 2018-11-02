// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/11/13)

#include "ConstantColorEffect.h"


ConstantColorEffect::ConstantColorEffect(eastl::shared_ptr<ProgramFactory> const& factory,
	eastl::vector<eastl::string> path, Vector4<float> const& color)
    :
    mPVWMatrix(nullptr),
    mColor(nullptr)
{
	eastl::string vsPath = path.front();
	eastl::string psPath = path.size() > 1 ? path[1] : path.front();
	eastl::string gsPath = path.size() > 2 ? path[2] : "";
	mProgram = factory->CreateFromFiles(vsPath, psPath, gsPath);
	if (mProgram)
	{
		mPVWMatrixConstant = eastl::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
		mPVWMatrix = mPVWMatrixConstant->Get<Matrix4x4<float>>();
		*mPVWMatrix = Matrix4x4<float>::Identity();

		mColorConstant = eastl::make_shared<ConstantBuffer>(sizeof(Vector4<float>), true);
		mColor = mColorConstant->Get<Vector4<float>>();
		*mColor = color;

		mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
		mProgram->GetVShader()->Set("ConstantColor", mColorConstant);
	}
}

void ConstantColorEffect::SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& pvwMatrix)
{
    mPVWMatrixConstant = pvwMatrix;
    mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}