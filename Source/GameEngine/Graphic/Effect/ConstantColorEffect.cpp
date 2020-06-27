// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2016/11/13)

#include "ConstantColorEffect.h"


ConstantColorEffect::ConstantColorEffect(eastl::shared_ptr<ProgramFactory> const& factory,
	eastl::vector<eastl::string> path, Vector4<float> const& color)
{
	eastl::string vsPath = path[0];
	eastl::string psPath = path[1];
	eastl::string gsPath = "";
	mProgram = factory->CreateFromFiles(vsPath, psPath, gsPath);
	if (mProgram)
	{
		mColorConstant = eastl::make_shared<ConstantBuffer>(sizeof(Vector4<float>), true);
		*mColorConstant->Get<Vector4<float>>() = color;

		mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
		mProgram->GetVShader()->Set("ConstantColor", mColorConstant);
	}
}

void ConstantColorEffect::SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& pvwMatrix)
{
    mPVWMatrixConstant = pvwMatrix;
    mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}