// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include "ColorEffect.h"

#include "Mathematic/Algebra/Vector2.h"

ColorEffect::ColorEffect(
	eastl::shared_ptr<ProgramFactory> const& factory, 
	eastl::vector<eastl::string> const& path)
	:
	mPVWMatrix(nullptr)
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

		mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
	}
}

void ColorEffect::SetPVWMatrixConstant(eastl::shared_ptr<ConstantBuffer> const& pvwMatrix)
{
	mPVWMatrixConstant = pvwMatrix;
	mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}